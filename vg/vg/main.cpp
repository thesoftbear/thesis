#include "application.hpp"
#include "particles.hpp"
#include "storage.hpp"
#include "grid.hpp"

int main()
{
	application a;

	particles p;
	//p.generate(10000, 0.001f);
	p.read("D:/laser.mmpld");
	
	grid g(128);
	g.update(p);

	while (a.step())
	{

	}

	return 0;
}