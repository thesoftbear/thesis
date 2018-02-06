#include "application.hpp"
#include "particles.hpp"
#include "storage.hpp"
#include "grid.hpp"

int main()
{
	application a;

	particles p;
	p.generate(1, 60000000, 0.001f);
	//p.read("C:/Users/Stacknit/Desktop/laser.mmpld");
	//p.select(0);

	unsigned int voxels = 128;
	unsigned int cells = 256;

	grid g(voxels);
	g.update_scattering(p);
	g.update_gathering(p, cells);

	while (a.step())
	{

	}

	return 0;
}