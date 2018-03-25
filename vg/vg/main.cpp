#include "application.hpp"
#include "particles.hpp"
#include "storage.hpp"
#include "voxelgrid.hpp"
#include "hashgrid.hpp"
#include "ambientocclusion.hpp"

#include <iostream>

const unsigned int voxels = 256;
const unsigned int cells = voxels / 2;
const float particle_radius = 0.01f;
const unsigned int particle_count = 25000;

int main()
{
	application a;

	particles p;
	p.generate(1, particle_count, particle_radius);
	//p.read("E:/laser.mmpld");
	//p.read("E:/oc_sim42_corrected.mmpld");
	// p.read("E:/exp2mill.mmpld");
	// p.select(0);

	hashgrid h(voxels / 8);
	h.insert(p);

	voxelgrid v(voxels);
	v.scatter(p);
	v.mipmap();

	ambientocclusion ao;

	while (true)
	{
		float start = a.elapsed();
		while (a.elapsed() - start < 10000.f)
		{
			ao.draw_geometry(a.elapsed(), p);

			for (unsigned int i = 0; i < 100; i++)
			{
				ao.cast_rays(h);
			}

			ao.draw_occlusion();

			a.step();
		}

		start = a.elapsed();
		while (a.elapsed() - start < 10000.f)
		{
			ao.draw_geometry(a.elapsed(), p);

			ao.trace_cones(v);

			ao.draw_occlusion();

			a.step();
		}
	}

	return 0;
}