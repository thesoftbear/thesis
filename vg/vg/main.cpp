#include "application.hpp"
#include "particles.hpp"
#include "storage.hpp"
#include "voxelgrid.hpp"
#include "hashgrid.hpp"
#include "ambientocclusion.hpp"

#include <iostream>

const unsigned int voxels = 256;
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

	hashgrid h(voxels / 4);
	h.insert(p);

	voxelgrid v(voxels);
	
	for (unsigned int i = 0; i < 10; i++)
	{
		v.scatter(p);
	}
	
	v.mipmap();

	ambientocclusion ao;
	
	ao.draw_geometry(a.elapsed(), p);
	
	while (a.step())
	{
		ao.cast_rays(h);
		
		//ao.trace_cones(v);

		ao.draw_occlusion();
	}

	return 0;
}