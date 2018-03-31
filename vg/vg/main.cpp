#include "application.hpp"
#include "particles.hpp"
#include "storage.hpp"
#include "voxelgrid.hpp"
#include "hashgrid.hpp"
#include "ambientocclusion.hpp"

#include <iostream>

const unsigned int voxels = 256;
const float particle_radius = 0.0025f;
const unsigned int particle_count = 300000;

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
	
	//for(unsigned int i = 0; i < 10; i++) v.scatter(h);
	
	for(unsigned int i = 0; i < 1; i++) v.scatterTexture2(h);

	v.mipmapTexture();

	ambientocclusion ao;
		
	while (a.step())
	{
		ao.update_geometry(a.get_state(), p);

		//for (unsigned int i = 0; i < 5; i++) ao.cast_rays(h);

		ao.trace_cones(v);

		ao.draw_occlusion();
	}

	return 0;
}