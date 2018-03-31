#pragma once

#include "application.hpp"
#include "voxelgrid.hpp"
#include "hashgrid.hpp"

#include <random>

#include "gtc\quaternion.hpp"

class ambientocclusion
{
	public:

		ambientocclusion();
		~ambientocclusion();
		void update_geometry(application_state s, particles & p); 
		void update_geometry(application_state s, hashgrid & h);
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
		
		float samples;

		GLuint framebuffer;
		GLuint renderbuffer;
		GLuint vao;

		float distance;
		quat orientation;
};