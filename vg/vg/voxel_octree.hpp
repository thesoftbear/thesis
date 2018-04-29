#pragma once

#include "particle_data.hpp"
#include "shader.hpp"
#include "hashgrid.hpp"

class voxel_octree
{
	public:

		voxel_octree();
		~voxel_octree();
		void set_resolution(unsigned int resolution);
		void save_level(string path, unsigned int level);
		void scatter_particles(particle_data & particles);
		void scatter_particles(space_partitioning_grid & grid);
		void scatter_particles_rasterizer(particle_data & particles);
		void scatter_particles_rasterizer(space_partitioning_grid & grid);
		void gather_particles(particle_data & particles);
		void gather_particles(space_partitioning_grid & grid);
		unsigned int get_resolution();
		GLuint get_texture();
		void clear_texture();

	private:

		float scatter_particles(unsigned int count, float radius, storage & positions);
		float scatter_particles_rasterizer(unsigned int count, float radius, storage & positions);
		float gather_particles(unsigned int count, float radius, storage & positions, unsigned int grid_resolution, storage & cell_info);
		void create_mipmap();
		void clamp_texture();

		unsigned int resolution;
		GLuint voxel_texture;
		shader scatter_shader;
		shader scatter_rasterizer_shader;
		shader gather_shader;
		shader mipmap_shader;
		shader clamp_shader;
};