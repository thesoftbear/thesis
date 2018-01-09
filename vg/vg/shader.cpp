#include "shader.hpp"
#include "error.hpp"

#include <vector>
#include <fstream>
#include <sstream>

shader::shader(string a)
{
	id = glCreateProgram();

	GLuint compiled_a = compile(a);
	
	glAttachShader(id, compiled_a);

	link();

	glDeleteShader(compiled_a);
}

shader::shader(string a, string b)
{
	id = glCreateProgram();

	GLuint compiled_a = compile(a);
	GLuint compiled_b = compile(b);

	glAttachShader(id, compiled_a);
	glAttachShader(id, compiled_b);

	link();

	glDeleteShader(compiled_a);
	glDeleteShader(compiled_b);
}

shader::~shader()
{
	glDeleteProgram(id);
}

void shader::set(string name, float value)
{
	GLint location = glGetUniformLocation(id, name.c_str());
	if (location == -1) error("uniform not found");
	else glUniform1f(location, value);
}

void shader::use()
{
	glUseProgram(id);
}

GLuint shader::compile(string s)
{
	GLenum type;

	string extension = s.substr(s.length() - 4, 4);
	if (extension == ".glcs") type = GL_COMPUTE_SHADER;
	else error("unsupported file");

	std::ifstream file(s);
	if (!file) error("path is no file");
	std::stringstream stream;
	stream << file.rdbuf();
	file.close();

	GLuint shader = glCreateShader(type);

	glShaderSource(shader, 1, (const GLchar * const *)stream.str().c_str(), 0);

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