#pragma once

#include "voxelgrid.hpp"
#include "hashgrid.hpp"

#include <random>

class ambientocclusion
{
	public:

		ambientocclusion();
		~ambientocclusion();
		void draw_geometry(float time, particles & p); 
		void draw_geometry(float time, hashgrid & h);
		void trace_cones(voxelgrid & v);
		void cast_rays(hashgrid & h);
		void draw_occlusion();

	private:

		shader draw_geometry_shader;
		shader trace_cones_shader;
		shader ray_casting;
		shader draw_occlusion_shader;

		default_random_engine generator;

		GLuint position_texture;
		GLuint normal_texture;
		GLuint occlusion_texture;
		
		unsigned int samples;

		GLuint framebuffer;
		GLuint renderbuffer;
		GLuint vao;
};