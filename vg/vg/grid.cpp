#include "grid.hpp"
#include <iostream>
#include "error.hpp"
#include <fstream>

grid::grid(unsigned int cells)
{
	_cells = cells;
	_data.set(sizeof(unsigned int) * cells * cells * cells);
}

grid::~grid()
{

}

void grid::update(particles & p)
{
	shader s("../vg/scattering.glcs");

	s.use();

	// upload cell size

	s.set("cell_size", 1.f / _cells);

	// upload particle size

	s.set("particle_size", p.size());

	// upload cell count

	s.set("cell_count", _cells);

	// upload particle count

	s.set("particle_count", p.number());

	// bind particle storage to 0

	p.data().bind(0);

	// bind grid storage to 1

	_data.bind(1);

	unsigned int groups = p.number() / 128 + 1;

	GLuint timer_queries[2];
	glGenQueries(2, timer_queries);

	glQueryCounter(timer_queries[0], GL_TIMESTAMP);

	s.execute(groups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glQueryCounter(timer_queries[1], GL_TIMESTAMP);

	GLuint64 time_0, time_1;
	glGetQueryObjectui64v(timer_queries[0], GL_QUERY_RESULT, &time_0);
	glGetQueryObjectui64v(timer_queries[1], GL_QUERY_RESULT, &time_1);

	std::cout << "scattering: " << double(time_1 - time_0) / 1000000 << "ms" << std::endl;

	// read back data;

	unsigned int * buffer = new unsigned int[_cells * _cells * _cells];
	_data.get(sizeof(unsigned int) * _cells * _cells * _cells, buffer);

	ofstream file("voxel" + to_string(_cells) + ".raw", ios::binary);
	file.write((char *)buffer, sizeof(unsigned int) * _cells * _cells * _cells);
	file.close();
}