#include "application.hpp"
#include "particle_data.hpp"
#include "storage.hpp"
#include "voxel_octree.hpp"
#include "hashgrid.hpp"
#include "ambient_occlusion.hpp"

#include <iostream>

const unsigned int voxels = 256;

int main()
{
	application a;

	particle_data p;
	// p.generate_random(100000, 0.005);
	// p.read_file("E:/laser.mmpld", 200);
	// p.read_file("E:/oc_sim42_corrected.mmpld", 3000);
	// p.read_file("E:/exp2mill.mmpld", 50);
	// p.read_file("E:/laser.00080.chkpt.density.mmpld", 0);

	p.generate_random(1000, 0.01);
	// p.read_file("C:/Users/Softbear/Desktop/laser.mmpld", 200);
	//p.read_file("E:/B2.X.nialout50.mmpld", 0);

	//p.generate_two(0.005, 0.01);

	space_partitioning_grid h;
	h.set_resolution(voxels / 4);
	h.insert_particles(p);

	voxel_octree v;
	v.set_resolution(voxels);
	v.scatter_particles(h);

	ambient_occlusion ao;
		
	bool ray_casting = false;

	while (a.step())
	{
		application_state s = a.get_state();

		ray_casting |= s.r_pressed;
		ray_casting &= !s.v_pressed;

		ao.update_geometry_buffer(s, p);

		if(ray_casting) ao.cast_rays(h);
		else ao.trace_cones(v, 1);
		
		ao.draw_occlusion();
	}

	return 0;
}