#pragma once

#include "voxelgrid.hpp"
#include "hashgrid.hpp"
#include "geometry_buffer.hpp"

#include <random>

class ambientocclusion
{
	public:

		ambientocclusion();
		void trace_cones(geometry_buffer & g, voxelgrid & v);
		void cast_rays(geometry_buffer & g, hashgrid & h);

	private:

		shader ray_casting;
};