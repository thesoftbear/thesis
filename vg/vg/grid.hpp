#pragma once

#include "particles.hpp"
#include "shader.hpp"

class grid
{
	public:

		grid(unsigned int voxels);
		~grid();
		void save(string path);
		void update_scattering(particles & p);
		void update_gathering(particles & p, unsigned int cells);
	
	private:

		unsigned int _voxels;
		storage _data;
};