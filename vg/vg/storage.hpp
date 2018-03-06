#pragma once

#include "glad\glad.h"

class storage
{
	public:

		storage();
		~storage();
		void set(unsigned int size, void * source = nullptr, GLenum usage = GL_DYNAMIC_DRAW);
		void get(unsigned int size, void * destination);
		void clear();
		unsigned int size();
		GLuint id();
		void bind(GLuint index);

	private:

		GLuint _id;
		unsigned int _size;
};