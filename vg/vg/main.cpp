#include "application.hpp"
#include "particles.hpp"
#include "storage.hpp"
#include "voxelgrid.hpp"
#include "hashgrid.hpp"
#include "ambientocclusion.hpp"

#include <iostream>

const unsigned int voxels = 256;
const unsigned int cells = voxels / 2;
const float particle_radius = 0.003f;
const unsigned int particle_count = 10000000;

int main()
{
	application a;

	particles p;
	p.generate(1, particle_count, particle_radius);
	//p.read("E:/laser.mmpld");
	//p.read("E:/oc_sim42_corrected.mmpld");
	// p.read("E:/exp2mill.mmpld");
	// p.select(0);

	voxelgrid v(voxels);

	hashgrid h(cells);
	
	ambientocclusion ao;

	while (a.step())
	{	
		h.resize(128);
		h.insert(p);
		v.scatter(h);

		/*
		for(unsigned int i = 0; i < 10; i++) v.scatter(p);
		for (unsigned int i = 0; i < 10; i++) v.scatter(h);

		h.resize(voxels / 2);
		for (unsigned int i = 0; i < 10; i++) h.insert(p);
		for (unsigned int i = 0; i < 10; i++) v.gather(h);
		 
		for (unsigned int i = 0; i < 10; i++) v.scatterTexture(p);
		*/
		// v.mipmap();

		ao.draw_geometry(a.elapsed(), p);
		ao.cast_rays(h);
	}

	return 0;
}