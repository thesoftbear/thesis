#pragma once

#include "glad\glad.h"

class geometry_buffer
{
	public:

		geometry_buffer(unsigned int width, unsigned int height);
		~geometry_buffer();
		GLuint get_position_texture();
		GLuint get_normal_texture();

	private:

		GLuint position_texture;
		GLuint normal_texture;

		// TODO add color buffer for output
};