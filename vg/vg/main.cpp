#include "application.hpp"
#include "particles.hpp"
#include "storage.hpp"
#include "voxelgrid.hpp"
#include "hashgrid.hpp"
#include "geometry_buffer.hpp"
#include "ambientocclusion.hpp"

#include <iostream>

const unsigned int voxels = 256;
const unsigned int cells = voxels / 2;
const float particle_radius = 0.003f;
const unsigned int particle_count = 20000000;

int main()
{
	application a;

	particles p;
	p.generate(1, particle_count, particle_radius);
	//p.read("E:/laser.mmpld");
	//p.read("E:/oc_sim42_corrected.mmpld");
	//p.select(0);

	std::cout << "particle size = " << p.size() << endl;

	voxelgrid v(voxels);

	hashgrid h(cells);

	//v.scatterTexture(p);
	//v.scatterTexture(p);
	//v.scatterTexture(p);

	while (a.step())
	{
		geometry_buffer gbuffer(1280, 720);

		//v.scatter(p);
		
		h.resize(voxels / 32);
		h.insert(p);
		v.scatter(h);
		
		//h.resize(voxels / 2);
		//h.insert(p);
		//v.gather(h);
		

		// v.mipmap();

		p.draw(a.elapsed(), gbuffer);

		ambientocclusion ao;

		ao.cast_rays(gbuffer, h);
	}

	return 0;
}