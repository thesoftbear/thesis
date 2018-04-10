#include "application.hpp"
#include "particles.hpp"
#include "storage.hpp"
#include "voxelgrid.hpp"
#include "hashgrid.hpp"
#include "ambientocclusion.hpp"

#include <iostream>

const unsigned int voxels = 256;
const float particle_radius = 0.01f;
const unsigned int particle_count = 50000;

int main()
{
	application a;

	particles p;
	p.generate(1, particle_count, particle_radius);
	//p.read("E:/laser.mmpld");
	// p.read("E:/oc_sim42_corrected.mmpld");
	//p.read("E:/exp2mill.mmpld");
	//p.read("E:/laser.00080.chkpt.density.mmpld");
	// p.select(0);

	hashgrid h(voxels / 4);
	h.insert(p);

	voxelgrid v(voxels);
	
	//for(unsigned int i = 0; i < 10; i++) v.scatter(h);
	
	for(unsigned int i = 0; i < 1; i++) v.scatterTexture2(h);

	//v.mipmapTexture();

	ambientocclusion ao;
		
	bool ray_casting = true;

	while (a.step())
	{
		application_state s = a.get_state();

		if (s.r_pressed && ray_casting == false)
		{
			// reselt ao
			ray_casting = true;
		}
		else if (s.v_pressed && ray_casting == true)
		{
			ray_casting = false;
		}

		ao.update_geometry(s, p);

		if(ray_casting) for (unsigned int i = 0; i < 3; i++) ao.cast_rays(h);
		else ao.trace_cones(v);

		ao.draw_occlusion();
	}

	return 0;
}