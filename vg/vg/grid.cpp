#include "grid.hpp"

grid::grid(unsigned int size)
{
	_size = size;
	_data.set(sizeof(unsigned int) * size * size * size);

	// compile shader
}

grid::~grid()
{

}

void grid::update(particles & p)
{
	shader s("../vg/scattering.glcs");

	s.use();

	// upload cell size to shader

	s.set("cell_size", 1.f / _size);

	// upload particle size to shader

	s.set("particle_size", p.size());

	// bind particle storage to 0

	p.data().bind(0);

	// bind grid storage to 1

	_data.bind(1);

	s.execute(1, 1, 1);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}