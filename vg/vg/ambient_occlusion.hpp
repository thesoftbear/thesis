#pragma once

#include "application.hpp"
#include "voxel_octree.hpp"
#include "hashgrid.hpp"

#include "gtc/quaternion.hpp"

#include <random>

class ambient_occlusion
{
	public:

		ambient_occlusion();
		~ambient_occlusion();
		void update_geometry_buffer(application_state s, particle_data & p); 
		void trace_cones(voxel_octree & v, unsigned int subdivision_factor);
		void cast_rays(space_partitioning_grid & h);
		void draw_occlusion();

	private:

		// shader programs 

		shader draw_geometry_shader;
		shader trace_cones_shader;
		shader cast_rays_shader;
		shader draw_occlusion_shader;

		// random generator for ray sample generation

		default_random_engine random_generator;

		// geometry buffer

		GLuint position_texture;
		GLuint normal_texture;
		
		// ambient occlusion buffer
		
		GLuint occlusion_texture;
		
		float samples;
		bool ray_casting;

		// opengl objects

		GLuint framebuffer;
		GLuint renderbuffer;
		GLuint vao;

		// view configuration

		float distance;
		quat orientation;
		vec2 resolution;

		bool update_view_configuration(application_state & s);
		
		std::vector<vec4> subdivide_hemisphere(unsigned int factor);
};