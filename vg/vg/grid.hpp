#pragma once

#include "particles.hpp"
#include "shader.hpp"

class grid
{
	public:

		grid(unsigned int cells);
		~grid();
		void update(particles & p);

	private:

		unsigned int _cells;
		storage _data;
};