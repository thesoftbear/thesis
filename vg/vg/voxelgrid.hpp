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
		void scatter(hashgrid & hashgrid);
		void gather(hashgrid & hashgrid);
		void scatterTexture(particles & particles);
		void scatterTexture(hashgrid & hashgrid);
		void mipmap();

	private:

		unsigned int _resolution;
		unsigned int _elements;
		storage _voxel_data;
		shader _scatter_unsorted;
		shader _scatter_sorted;
		shader _scatter_texture;
		shader _gather;
		shader _mipmap;

};