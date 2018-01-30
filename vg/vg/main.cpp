#include "application.hpp"
#include "particles.hpp"
#include "storage.hpp"
#include "grid.hpp"

int main()
{
	application a;

	particles p;
	//p.generate(10000, 0.001f);
	p.read("C:/Users/Stacknit/Desktop/laser.mmpld");
	p.select(0);

	grid g(32);
	g.update(p);

	while (a.step())
	{

	}

	return 0;
}