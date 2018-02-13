#pragma once

#include "particles.hpp"
#include "shader.hpp"
#include "hashgrid.hpp"

class voxelgrid
{
	public:

		voxelgrid(unsigned int resolution);
		~voxelgrid();
		void save(string path);
		void scatter(particles & particles);
		void gather(hashgrid & hashgrid);
		void get();
	
	private:

		unsigned int _resolution;
		storage _voxel_data;
		shader _scatter;
		shader _gather;
};