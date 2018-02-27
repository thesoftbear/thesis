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
		void scatter3DTexture(particles & particles);
		void get();
		void mipmap();

	private:

		unsigned int _resolution;
		unsigned int _elements;
		storage _voxel_data;
		shader _scatter;
		shader _gather;
		shader _mipmap;



		shader _scatter3DTexture;





		float scattering_time[10];
		float scattering_sum;
		unsigned int scattering_iteration;
		float gathering_time[10];
		float gathering_sum;
		unsigned int gathering_iteration;
};