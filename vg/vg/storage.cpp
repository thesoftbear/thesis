#include "storage.hpp"
#include "error.hpp"

storage::storage()
{
	glGenBuffers(1, &_id);

	if (glGetError()) error("storage creation failed");
}

storage::~storage()
{
	glDeleteBuffers(1, &_id);

	if (glGetError()) error("storage deletion failed");
}

void storage::set(unsigned int size, void * source, GLenum usage)
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, _id);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, source, usage);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	if (glGetError()) error("storage set failed");
}

void storage::get(unsigned int size, void * destination)
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, _id);
	void * buffer = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	memcpy(destination, buffer, size);
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	if (glGetError()) error("storage get failed");
}

GLuint storage::id()
{
	return _id;
}

void storage::bind(GLuint index)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, _id);
}