#include "grid.hpp"
#include <iostream>
#include "error.hpp"
#include <fstream>
#include <chrono>

grid::grid(unsigned int voxels)
{
	_voxels = voxels;
	_data.set(sizeof(unsigned int) * voxels * voxels * voxels);
}

grid::~grid()
{

}

void grid::save(string path)
{
	unsigned int elements = _voxels * _voxels * _voxels;
	unsigned int * buffer = new unsigned int[elements];
	_data.get(sizeof(unsigned int) * elements, buffer);

	ofstream file(path, ios::binary);
	file.write((char *)buffer, sizeof(unsigned int) * elements);
	file.close();

	delete[] buffer;
}

void grid::update_scattering(particles & p)
{
		shader s("../vg/scattering.glcs");

		s.use();

	// upload cell size

		s.set("cell_size", 1.f / _voxels);

	// upload particle size

		s.set("particle_size", p.size());

	// upload cell count

		s.set("cell_count", _voxels);

	// upload particle count

		s.set("particle_count", p.number());

	// bind particle storage to 0

		p.data().bind(0);

	// bind grid storage to 1

		_data.bind(1);

		GLuint timer_queries[2];
		glGenQueries(2, timer_queries);
		glQueryCounter(timer_queries[0], GL_TIMESTAMP);

		unsigned int groups = ceil(p.number() / 1024.f);
		s.execute(groups);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glQueryCounter(timer_queries[1], GL_TIMESTAMP);
		GLuint64 time_0, time_1;
		glGetQueryObjectui64v(timer_queries[0], GL_QUERY_RESULT, &time_0);
		glGetQueryObjectui64v(timer_queries[1], GL_QUERY_RESULT, &time_1);

		cout << "[s] v: " << _voxels << " s: " << double(time_1 - time_0) / 1000000 << " ms" << endl;

		save("scattering_v" + to_string(_voxels) + ".raw");
}

/*
hashing.glcs:
0: particles
1: uint cell -> per particle cell
2: uint index -> per particle position inside cell
3: uint count -> per cell count of particles inside

prefixsum.glcs:
3: uint count ...
4: uint prefix sum -> per cell start address
5: uint global counter -> single

sorting.glcs:
0: particles
1: cell
2: index
4: prefix sum
6: sorted -> sorted particles

gathering.glcs:
3: uint count
4: prefix sum
6: sorted particles
7: densities

*/

void grid::update_gathering(particles & p, unsigned int cells)
{
	cout << "[g] v: " << _voxels << " a: " << cells;


	unsigned int _cells = cells;

	// create auxillary storage

	storage particle_cell;
	particle_cell.set(p.number() * sizeof(unsigned int));

	storage particle_index;
	particle_index.set(p.number() * sizeof(unsigned int));

	storage cell_count;
	cell_count.set(_cells * _cells * _cells * sizeof(unsigned int));

	storage cell_address;
	cell_address.set(_cells * _cells * _cells * sizeof(unsigned int));

	storage prefix_data;
	unsigned int zero[3] = { 0, 0, 0 };
	prefix_data.set(3 * sizeof(unsigned int), &zero[0]);

	storage sorted_particles;
	sorted_particles.set(p.number() * 3 * sizeof(float));

	// hash particles

		shader hashing("../vg/hashing.glcs");
		hashing.use();

		hashing.set("particle_count", p.number());
		hashing.set("cell_count", _cells);
		p.data().bind(0);
		particle_cell.bind(1);
		particle_index.bind(2);
		cell_count.bind(3);

		unsigned int groups = ceil(p.number() / 1024.f);

		GLuint timer_queries[2];
		glGenQueries(2, timer_queries);
		glQueryCounter(timer_queries[0], GL_TIMESTAMP);

		hashing.execute(groups);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glQueryCounter(timer_queries[1], GL_TIMESTAMP);
		GLuint64 time_0, time_1;
		glGetQueryObjectui64v(timer_queries[0], GL_QUERY_RESULT, &time_0);
		glGetQueryObjectui64v(timer_queries[1], GL_QUERY_RESULT, &time_1);

		float time = 0.f;
		time += double(time_1 - time_0) / 1000000;
		cout << " h: " << double(time_1 - time_0) / 1000000 << " ms";


	// prefix sum

		/*
		shader prefixsum("../vg/prefixsum.glcs");
		prefixsum.use();

		cell_count.bind(3);
		cell_address.bind(4);
		prefix_data.bind(5);

		groups = ceil((_cells * _cells * _cells) / (256.f * 16));

		glGenQueries(2, timer_queries);
		glQueryCounter(timer_queries[0], GL_TIMESTAMP);

		prefixsum.execute(groups);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glQueryCounter(timer_queries[1], GL_TIMESTAMP);
		glGetQueryObjectui64v(timer_queries[0], GL_QUERY_RESULT, &time_0);
		glGetQueryObjectui64v(timer_queries[1], GL_QUERY_RESULT, &time_1);

		std::cout << "prefix sum: " << double(time_1 - time_0) / 1000000 << " ms (" << groups << " work groups)" << std::endl;
		*/
		
		chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();

		unsigned int * buffer = new unsigned int[_cells * _cells * _cells];
		cell_count.get(_cells * _cells * _cells * sizeof(unsigned int), buffer);

		unsigned int sum = 0;
		unsigned int last = 0;

		for (unsigned int i = 0; i < _cells * _cells * _cells; i++)
		{
			sum += buffer[i];
			buffer[i] = last;
			last = sum;
		}

		cell_address.set(_cells * _cells * _cells * sizeof(unsigned int), buffer);
		delete[] buffer;

		chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();

		time += chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000.f;
		cout << " p: " << chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000.f << " ms";

	// sort particles
		
		shader sorting("../vg/sorting.glcs");
		sorting.use();

		p.data().bind(0);
		particle_cell.bind(1);
		particle_index.bind(2);
		cell_address.bind(4);
		sorted_particles.bind(6);
		sorting.set("particle_count", p.number());

		groups = ceil(p.number() / 1024.f);

		glGenQueries(2, timer_queries);
		glQueryCounter(timer_queries[0], GL_TIMESTAMP);

		sorting.execute(groups);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glQueryCounter(timer_queries[1], GL_TIMESTAMP);
		glGetQueryObjectui64v(timer_queries[0], GL_QUERY_RESULT, &time_0);
		glGetQueryObjectui64v(timer_queries[1], GL_QUERY_RESULT, &time_1);

		time += double(time_1 - time_0) / 1000000;
		cout << " s: " << double(time_1 - time_0) / 1000000 << " ms";

	// gather voxels

		shader gathering("../vg/gathering.glcs");
		gathering.use();

		cell_count.bind(3);
		cell_address.bind(4);
		sorted_particles.bind(6);
		_data.bind(7);
		float voxel_size = 1.f / _voxels;
		float voxel_size_squared = voxel_size * voxel_size;
		float voxel_radius = sqrt(voxel_size_squared + voxel_size_squared + voxel_size_squared) / 2;
		gathering.set("voxel_size", voxel_size);
		gathering.set("voxel_count", _voxels);
		//gathering.set("cell_size", 1.f / _cells);
		gathering.set("cell_count", _cells);
		gathering.set("particle_size", p.size());
		gathering.set("voxel_radius", voxel_radius),
		groups = ceil(_voxels / 8.f);

		glGenQueries(2, timer_queries);
		glQueryCounter(timer_queries[0], GL_TIMESTAMP);

		gathering.execute(groups, groups, groups);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glQueryCounter(timer_queries[1], GL_TIMESTAMP);
		glGetQueryObjectui64v(timer_queries[0], GL_QUERY_RESULT, &time_0);
		glGetQueryObjectui64v(timer_queries[1], GL_QUERY_RESULT, &time_1);

		time += double(time_1 - time_0) / 1000000;
		cout << " g: " << double(time_1 - time_0) / 1000000 << " ms -> " << time << " ms" << endl;


		save("gathering_v" + to_string(_voxels) + "_g" + to_string(cells) + ".raw");
}