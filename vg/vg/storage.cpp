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

void storage::set(unsigned int size, void * data, GLenum usage)
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, _id);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, usage);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	if (glGetError()) error("storage data failed");
}

GLuint storage::id()
{
	return _id;
}

void storage::bind(GLuint index)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, _id);
}