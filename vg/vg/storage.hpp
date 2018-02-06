#pragma once
#include "glad\glad.h"

class storage
{
	public:

		storage();
		~storage();
		void set(unsigned int size, void * source = nullptr, GLenum usage = GL_DYNAMIC_DRAW);
		void get(unsigned int size, void * destination);

		GLuint id();
		void bind(GLuint index);

	private:

		GLuint _id;
};