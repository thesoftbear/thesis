#pragma once

#include "particle_data.hpp"
#include "shader.hpp"

class space_partitioning_grid
{
	public:
	
		space_partitioning_grid();
		void set_resolution(unsigned int resolution);
		void insert_particles(particle_data & particles);
		storage & get_cell_info();
		storage & get_particle_storage();
		unsigned int get_resolution();
		float get_particle_radius();
		unsigned int get_particle_count();
		void verify_partitioning(particle_data & particles);

	private:

		void hash_particles(particle_data & particles, storage & particle_info);
		void calculate_addresses();
		void sort_particles(particle_data & particles, storage & particle_info);
		unsigned int resolution;
		storage cell_info;
		storage particle_storage;
		float particle_radius;
		unsigned int particle_count;
		shader hashing_shader;
		shader prefixsum_shader;
		shader sorting_shader;
};