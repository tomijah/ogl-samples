///////////////////////////////////////////////////////////////////////////////////
/// OpenGL Samples Pack (ogl-samples.g-truc.net)
///
/// Copyright (c) 2004 - 2014 G-Truc Creation (www.g-truc.net)
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
///////////////////////////////////////////////////////////////////////////////////

#include "test.hpp"

namespace
{
	char const * VERT_SHADER_SOURCE("gl-320/buffer-uniform.vert");
	char const * FRAG_SHADER_SOURCE("gl-320/buffer-uniform.frag");

	struct vertex_v3fn3fc4f
	{
		vertex_v3fn3fc4f
		(
			glm::vec3 const & Position,
			glm::vec3 const & Texcoord,
			glm::vec4 const & Color
		) :
			Position(Position),
			Texcoord(Texcoord),
			Color(Color)
		{}

		glm::vec3 Position;
		glm::vec3 Texcoord;
		glm::vec4 Color;
	};

	GLsizei const VertexCount(4);
	GLsizeiptr const VertexSize = VertexCount * sizeof(vertex_v3fn3fc4f);
/*
	glf::vertex_v3fn3f const VertexData[VertexCount] =
	{
		glf::vertex_v3fn3f(glm::vec3(-1.0f,-1.0f, 0.0), glm::vec3(0.0f, 0.0f, 1.0f)),
		glf::vertex_v3fn3f(glm::vec3( 1.0f,-1.0f, 0.0), glm::vec3(0.0f, 0.0f, 1.0f)),
		glf::vertex_v3fn3f(glm::vec3( 1.0f, 1.0f, 0.0), glm::vec3(0.0f, 0.0f, 1.0f)),
		glf::vertex_v3fn3f(glm::vec3(-1.0f, 1.0f, 0.0), glm::vec3(0.0f, 0.0f, 1.0f))
	};
*/
	vertex_v3fn3fc4f const VertexData[VertexCount] =
	{
		vertex_v3fn3fc4f(glm::vec3(-1.000f, -0.732f, -0.732f), glm::normalize(glm::vec3(-1.000f, -0.732f, -0.732f)), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)),
		vertex_v3fn3fc4f(glm::vec3( 1.000f, -0.732f, -0.732f), glm::normalize(glm::vec3( 1.000f, -0.732f, -0.732f)), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)),
		vertex_v3fn3fc4f(glm::vec3( 0.000f,  1.000f, -0.732f), glm::normalize(glm::vec3( 0.000f,  1.000f, -0.732f)), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)),
		vertex_v3fn3fc4f(glm::vec3( 0.000f,  0.000f,  1.000f), glm::normalize(glm::vec3( 0.000f,  0.000f,  1.000f)), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f))
	};

	GLsizei const ElementCount(12);
	GLsizeiptr const ElementSize = ElementCount * sizeof(GLushort);
	GLushort const ElementData[ElementCount] =
	{
		0, 2, 1,
		0, 1, 3,
		1, 2, 3,
		2, 0, 3
	};

	namespace buffer
	{
		enum type
		{
			VERTEX,
			ELEMENT,
			TRANSFORM,
			MATERIAL,
			LIGHT,
			MAX
		};
	}//namespace buffer

	namespace uniform
	{
		enum type
		{
			MATERIAL = 0,
			TRANSFORM0 = 1,
			PER_SCENE = 2,
			LIGHT = 3
		};
	};

	struct material
	{
		glm::vec4 Ambient;
		glm::vec4 Diffuse;
		glm::vec4 Specular;
		float Shininess;
	};

	struct light
	{
		glm::vec3 Position;
	};

	struct transform
	{
		glm::mat4 P;
		glm::mat4 MV;
		glm::mat3 Normal;
	};
}//namespace

class instance : public test
{
public:
	instance(int argc, char* argv[]) :
		test(argc, argv, "gl-320-buffer-uniform", test::CORE, 3, 2),
		VertexArrayName(0),
		ProgramName(0),
		UniformTransform(0),
		UniformMaterial(0),
		UniformPerScene(0)
	{}

private:
	std::array<GLuint, buffer::MAX> BufferName;
	GLuint ProgramName;
	GLuint VertexArrayName;
	GLint UniformTransform;
	GLint UniformMaterial;
	GLint UniformPerScene;
	GLint UniformLight;

	bool initProgram()
	{
		bool Validated = true;

		compiler Compiler;
	
		// Create program
		if(Validated)
		{
			compiler Compiler;
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE, "--version 150 --profile core");
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE, "--version 150 --profile core");

			ProgramName = glCreateProgram();
			glAttachShader(ProgramName, VertShaderName);
			glAttachShader(ProgramName, FragShaderName);

			glBindAttribLocation(ProgramName, semantic::attr::POSITION, "Position");
			glBindAttribLocation(ProgramName, semantic::attr::NORMAL, "Normal");
			glBindAttribLocation(ProgramName, semantic::attr::COLOR, "Color");
			glBindFragDataLocation(ProgramName, semantic::frag::COLOR, "Color");
			glLinkProgram(ProgramName);

			Validated = Validated && Compiler.check();
			Validated = Validated && Compiler.checkProgram(ProgramName);
		}

		// Get variables locations
		if(Validated)
		{
			this->UniformTransform = glGetUniformBlockIndex(ProgramName, "transform");
			this->UniformMaterial = glGetUniformBlockIndex(ProgramName, "material");
			this->UniformLight = glGetUniformBlockIndex(ProgramName, "light");

			glUniformBlockBinding(ProgramName, UniformTransform, uniform::TRANSFORM0);
			glUniformBlockBinding(ProgramName, UniformMaterial, uniform::MATERIAL);
			glUniformBlockBinding(ProgramName, UniformLight, uniform::LIGHT);
		}
	
		return Validated;
	}

	bool initVertexArray()
	{
		glGenVertexArrays(1, &VertexArrayName);
		glBindVertexArray(VertexArrayName);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
			glVertexAttribPointer(semantic::attr::POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_v3fn3fc4f), BUFFER_OFFSET(0));
			glVertexAttribPointer(semantic::attr::NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_v3fn3fc4f), BUFFER_OFFSET(sizeof(glm::vec3)));
			glVertexAttribPointer(semantic::attr::COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_v3fn3fc4f), BUFFER_OFFSET(sizeof(glm::vec3) + sizeof(glm::vec3)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glEnableVertexAttribArray(semantic::attr::POSITION);
			glEnableVertexAttribArray(semantic::attr::NORMAL);
			glEnableVertexAttribArray(semantic::attr::COLOR);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
		glBindVertexArray(0);

		return true;
	}

	bool initBuffer()
	{
		// Generate buffer objects
		glGenBuffers(buffer::MAX, &BufferName[0]);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
		glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		{
			glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(transform), nullptr, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		{
			light Light = {glm::vec3(0.0f, 0.0f, 100.f)};

			glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::LIGHT]);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(Light), &Light, GL_STATIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		{
			material Material = {glm::vec4(0.2f, 0.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.5f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 0.5f, 1.0f), 4.0f};

			glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::MATERIAL]);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(Material), &Material, GL_STATIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		return true;
	}

	bool begin()
	{
		bool Validated = true;

		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initVertexArray();

		glEnable(GL_DEPTH_TEST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDrawBuffer(GL_BACK);
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			return false;

		return Validated;
	}

	bool end()
	{
		glDeleteVertexArrays(1, &VertexArrayName);
		glDeleteBuffers(buffer::MAX, &BufferName[0]);
		glDeleteProgram(ProgramName);

		return true;
	}

	bool render()
	{
		glm::vec2 WindowSize(this->getWindowSize());

		{
			glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
			transform* Transform = static_cast<transform*>(glMapBufferRange(GL_UNIFORM_BUFFER,
				0, sizeof(transform), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));

			glm::mat4 const Projection = glm::perspective(glm::pi<float>() * 0.25f, 4.0f / 3.0f, 0.1f, 100.0f);
			glm::mat4 const View = this->view();
			glm::mat4 const Model = glm::rotate(glm::mat4(1.0f), -glm::pi<float>() * 0.5f, glm::vec3(1.0f, 0.0f, 0.0f));
		
			Transform->MV = View * Model;
			Transform->P = Projection;
			Transform->Normal = glm::mat3(glm::transpose(glm::inverse(Transform->MV)));

			glUnmapBuffer(GL_UNIFORM_BUFFER);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		glViewport(0, 0, static_cast<GLsizei>(WindowSize.x), static_cast<GLsizei>(WindowSize.y));
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.2f, 0.2f, 0.2f, 1.0f)[0]);
		glClearBufferfv(GL_DEPTH, 0, &glm::vec1(1.0f)[0]);

		glUseProgram(ProgramName);

		glBindBufferBase(GL_UNIFORM_BUFFER, uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);
		glBindBufferBase(GL_UNIFORM_BUFFER, uniform::MATERIAL, BufferName[buffer::MATERIAL]);
		glBindBufferBase(GL_UNIFORM_BUFFER, uniform::LIGHT, BufferName[buffer::LIGHT]);

		glBindVertexArray(VertexArrayName);
		glDrawElementsInstancedBaseVertex(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, nullptr, 1, 0);
		//glDrawElementsInstancedBaseVertex(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, BUFFER_OFFSET(sizeof(GLushort) * 3), 1, 0);

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	instance Test(argc, argv);
	Error += Test();

	return Error;
}
