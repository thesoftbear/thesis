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

	private:

		shader draw_geometry_shader;
		shader trace_cones_shader;
		shader ray_casting;

		default_random_engine generator;

		GLuint position_texture;
		GLuint normal_texture;
		float model_rotation;
};