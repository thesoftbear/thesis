#include "application.hpp"
#include "particles.hpp"
#include "storage.hpp"
#include "grid.hpp"

int main()
{
	application a;

	particles p;
	p.generate(10000000, 0.01f);

	grid g(128);
	g.update(p);

	while (a.step())
	{

	}

	return 0;
}