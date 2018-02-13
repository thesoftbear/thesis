#pragma once

#include "particles.hpp"
#include "shader.hpp"

class hashgrid
{
	public:
	
		hashgrid(unsigned int resolution);
		void resize(unsigned int resolution);
		void insert(particles & particles);
		void get(unsigned int & resolution, storage * & info, storage * & particles);

	private:

		unsigned int _resolution;
		storage _cell_info;
		storage _particle_info;
		storage _sorted_particles;
		storage _prefix_data;

		shader _hashing;
		shader _prefixsum;
		shader _sorting;
};