#include "application.hpp"
#include "particles.hpp"
#include "storage.hpp"
#include "voxelgrid.hpp"
#include "hashgrid.hpp"

#include <iostream>

const unsigned int voxels = 256;
const unsigned int cells = voxels / 2;
const float particle_radius = 0.005f;
const unsigned int particle_count = 5500000;

/*
void morton()
{
	unsigned int offsets[8] = { 1, 8, 64, 512, 4096, 32768, 262144, 2097152 };

	unsigned int morton[256];

	for (unsigned int x = 0; x < 256; x++)
	{
		unsigned int remainder = x;
		unsigned int offset = 0;

		if (remainder >= 128)
		{
			remainder %= 128;
			offset += offsets[7];
		}

		if (remainder >= 64)
		{
			remainder %= 64;
			offset += offsets[6];
		}

		if (remainder >= 32)
		{
			remainder %= 32;
			offset += offsets[5];
		}

		if (remainder >= 16)
		{
			remainder %= 16;
			offset += offsets[4];
		}

		if (remainder >= 8)
		{
			remainder %= 8;
			offset += offsets[3];
		}

		if (remainder >= 4)
		{
			remainder %= 4;
			offset += offsets[2];
		}

		if (remainder >= 2)
		{
			remainder %= 2;
			offset += offsets[1];
		}

		if (remainder >= 1)
		{
			remainder %= 1;
			offset += offsets[0];
		}

		morton[x] = offset;

		cout << morton[x] << ", ";
	}
}
*/

int main()
{
	application a;

	particles p;
	p.generate(1, particle_count, particle_radius);
	//p.read("E:/laser.mmpld");
	//p.read("E:/oc_sim42_corrected.mmpld");
	//p.select(0);

	voxelgrid v(voxels);

	hashgrid h(cells);

	while (a.step())
	{
		v.scatter(p);

		h.insert(p);
		v.gather(h);

		// v.save("test.raw");
	}

	return 0;
}