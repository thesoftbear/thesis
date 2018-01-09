#pragma once

#include "particles.hpp"
#include "shader.hpp"

class grid
{
	public:

		grid(unsigned int size);
		~grid();
		void update(particles & p);

	private:

		unsigned int _size;
		storage _data;
};