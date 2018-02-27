#include "application.hpp"
#include "particles.hpp"
#include "storage.hpp"
#include "voxelgrid.hpp"
#include "hashgrid.hpp"

#include <iostream>

const unsigned int voxels = 256;
const unsigned int cells = voxels / 8;
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

	h.insert(p);

	while (a.step())
	{
		for(unsigned int i = 0; i < 10; i++) v.scatter(p);

		for (unsigned int i = 0; i < 10; i++) v.gather(h);

		// v.mipmap();

		// v.save("scatter2.raw");

		// p.draw();

		v.scatter3DTexture(p);
	}

	return 0;
}