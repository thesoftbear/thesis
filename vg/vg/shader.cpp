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
	if (location == -1) error("uniform " + name + " not found");
	else glUniform1f(location, value);
}

void shader::set(string name, unsigned int value)
{
	GLint location = glGetUniformLocation(id, name.c_str());
	if (location == -1) error("uniform " + name + " not found");
	else glUniform1ui(location, value);
}

void shader::use()
{
	glUseProgram(id);
}

void shader::execute(unsigned int x, unsigned int y, unsigned int z)
{
	glDispatchCompute(x, y, z);
}

GLuint shader::compile(string path)
{
	GLenum type;

	string extension = path.substr(path.length() - 5, 5);
	if (extension == ".glcs") type = GL_COMPUTE_SHADER;
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