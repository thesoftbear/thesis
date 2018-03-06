#pragma once

#include "particles.hpp"
#include "shader.hpp"

class hashgrid
{
	public:
	
		hashgrid(unsigned int resolution);
		void resize(unsigned int resolution);
		void insert(particles & particles);
		storage & get_cell_info();
		storage & get_particle_data();
		unsigned int get_resolution();
		float get_particle_size();
		unsigned int get_particle_number();

	private:

		unsigned int _resolution;
		storage _cell_info;
		storage _particle_info;
		storage _sorted_particles;
		storage _prefix_data;
		float particle_size;
		unsigned int particle_number;
		shader _hashing;
		shader _prefixsum;
		shader _sorting;
};