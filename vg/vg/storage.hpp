#pragma once
#include "glad\glad.h"

class storage
{
	public:

		storage();
		~storage();
		void set(unsigned int size, void * data = nullptr, GLenum usage = GL_STATIC_DRAW);
		GLuint id();
		void bind(GLuint index);

	private:

		GLuint _id;
};