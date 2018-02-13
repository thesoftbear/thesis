#include "application.hpp"
#include "particles.hpp"
#include "storage.hpp"
#include "voxelgrid.hpp"
#include "hashgrid.hpp"

int main()
{
	application a;

	particles p;
	p.generate(1, 20000000, 0.008f);
	//p.read("E:/laser.mmpld");
	//p.read("E:/oc_sim42_corrected.mmpld");
	//p.select(0);

	unsigned int voxels = 256;
	unsigned int cells = 128;

	voxelgrid v(voxels);

	hashgrid h(cells);

	while (a.step())
	{
		v.scatter(p);

		h.insert(p);
		v.gather(h);
	}

	return 0;
}