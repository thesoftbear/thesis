#include "shader.hpp"
#include "error.hpp"

#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

shader::shader()
{
	id = glCreateProgram();
}

shader::~shader()
{
	glDeleteProgram(id);
}

void shader::source(string a)
{
	GLuint compiled_a = compile(a);

	glAttachShader(id, compiled_a);

	link();

	glDeleteShader(compiled_a);

	compute = true;
}

void shader::source(string a, string b)
{
	GLuint compiled_a = compile(a);
	GLuint compiled_b = compile(b);

	glAttachShader(id, compiled_a);
	glAttachShader(id, compiled_b);

	link();

	glDeleteShader(compiled_a);
	glDeleteShader(compiled_b);

	compute = false;
}

void shader::source(string a, string b, string c)
{
	GLuint compiled_a = compile(a);
	GLuint compiled_b = compile(b);
	GLuint compiled_c = compile(c);

	glAttachShader(id, compiled_a);
	glAttachShader(id, compiled_b);
	glAttachShader(id, compiled_c);

	link();

	glDeleteShader(compiled_a);
	glDeleteShader(compiled_b);
	glDeleteShader(compiled_c);

	compute = false;
}

void shader::set(string name, float value)
{
	GLint location = glGetUniformLocation(id, name.c_str());
	glUniform1f(location, value);
}

void shader::set(string name, unsigned int value)
{
	GLint location = glGetUniformLocation(id, name.c_str());
	glUniform1ui(location, value);
}

void shader::set(string name, mat4 value)
{
	GLint location = glGetUniformLocation(id, name.c_str());
	glUniformMatrix4fv(location, 1, false, &value[0][0]);
}

void shader::set(string name, int value)
{
	GLint location = glGetUniformLocation(id, name.c_str());
	glUniform1i(location, value);
}

void shader::set(string name, unsigned int count, unsigned int * values)
{
	GLint location = glGetUniformLocation(id, name.c_str());
	glUniform1uiv(location, count, values);
}

void shader::set(string name, vec2 value)
{
	GLint location = glGetUniformLocation(id, name.c_str());
	glUniform2f(location, value.x, value.y);
}

void shader::use()
{
	glUseProgram(id);
}

void shader::execute(unsigned int x, unsigned int y, unsigned int z)
{
	if (compute) glDispatchCompute(x, y, z);
	else glDrawArrays(x, y, z);

	size_t e = glGetError();
	if (e != GL_NO_ERROR) error("shader execution " + to_string(e));
}

GLuint shader::compile(string path)
{
	GLenum type;

	string extension = path.substr(path.length() - 5, 5);
	if (extension == ".glcs") type = GL_COMPUTE_SHADER;
	else if (extension == ".glvs") type = GL_VERTEX_SHADER;
	else if (extension == ".glfs") type = GL_FRAGMENT_SHADER;
	else if (extension == ".glgs") type = GL_GEOMETRY_SHADER;
	else error("unsupported file");

	ifstream file(path);
	if (!file.good()) error("path is no file");
	stringstream stream;
	stream << file.rdbuf();
	file.close();

	GLuint shader = glCreateShader(type);
	string str = stream.str();
	const char * c_str = str.c_str();
	glShaderSource(shader, 1, &c_str, NULL);
	
	glCompileShader(shader);

	GLint result = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

	if (result == GL_FALSE)
	{
		GLint length = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

		std::vector<GLchar> info(length);
		glGetShaderInfoLog(shader, length, &length, info.data());

		error(string(info.begin(), info.end()));
	}

	return shader;
}

void shader::link()
{
	glLinkProgram(id);

	GLint result = 0;
	glGetProgramiv(id, GL_LINK_STATUS, &result);

	if (result == GL_FALSE)
	{
		GLint length = 0;
		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &length);

		std::vector<GLchar> info(length);
		glGetProgramInfoLog(id, length, &length, info.data());

		error(string(info.begin(), info.end()));
	}
}