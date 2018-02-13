#include "hashgrid.hpp"

#include <iostream>
#include <chrono>

hashgrid::hashgrid(unsigned int resolution)
{
	_resolution = 0;
	resize(resolution);

	_hashing.source("../vg/hashing.glcs");
	_prefixsum.source("../vg/prefixsum.glcs");
	_sorting.source("../vg/sorting.glcs");

	_prefix_data.set(3 * sizeof(unsigned int), nullptr, GL_DYNAMIC_COPY);
	_prefix_data.clear();
}

void hashgrid::resize(unsigned int resolution)
{
	if (_resolution != resolution)
	{
		_resolution = resolution;

		_cell_info.set(resolution * resolution * resolution * 2 * sizeof(unsigned int), nullptr, GL_DYNAMIC_COPY);
		_cell_info.clear();
	}
}

void hashgrid::insert(particles & particles)
{
	// allocate storage according to number of particles

		if (_sorted_particles.size() != particles.data().size())
		{
			_sorted_particles.set(particles.data().size(), nullptr, GL_DYNAMIC_COPY);

			_particle_info.set(particles.number() * 2 * sizeof(unsigned int), nullptr, GL_DYNAMIC_COPY);
			_particle_info.clear();
		}

	// clear old cell info

		_cell_info.clear();
		
	// spatial hashing

		_hashing.use();

		_hashing.set("particle_count", particles.number());
		_hashing.set("cell_count", _resolution);
		particles.data().bind(0);
		_particle_info.bind(1);
		_cell_info.bind(2);

		unsigned int groups = ceil(particles.number() / 1.f);

		GLuint timer_queries[2];
		glGenQueries(2, timer_queries);
		glQueryCounter(timer_queries[0], GL_TIMESTAMP);

		_hashing.execute(groups);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glQueryCounter(timer_queries[1], GL_TIMESTAMP);
		GLuint64 time_0, time_1;
		glGetQueryObjectui64v(timer_queries[0], GL_QUERY_RESULT, &time_0);
		glGetQueryObjectui64v(timer_queries[1], GL_QUERY_RESULT, &time_1);

		float time = 0.f;
		time += double(time_1 - time_0) / 1000000;
		cout << "[gathering] hashing: " << double(time_1 - time_0) / 1000000 << " ms";

	// prefix sum
		
		_prefix_data.clear();

		_prefixsum.use();

		_cell_info.bind(2);
		_prefix_data.bind(3);

		groups = ceil((_resolution * _resolution * _resolution) / (32 * 16.f));

		glGenQueries(2, timer_queries);
		glQueryCounter(timer_queries[0], GL_TIMESTAMP);

		_prefixsum.execute(groups);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glQueryCounter(timer_queries[1], GL_TIMESTAMP);
		glGetQueryObjectui64v(timer_queries[0], GL_QUERY_RESULT, &time_0);
		glGetQueryObjectui64v(timer_queries[1], GL_QUERY_RESULT, &time_1);

		std::cout << " prefix: " << double(time_1 - time_0) / 1000000 << " ms";
	
	// verify prefix sum
		
		/*
		unsigned int * buffer = new unsigned int[_resolution * _resolution * _resolution * 2];
		_cell_info.get(_resolution * _resolution * _resolution * 2 * sizeof(unsigned int), buffer);

		unsigned int sum = 0;
		unsigned int errors = 0;

		for (unsigned int i = 0; i < _resolution * _resolution * _resolution * 2; i += 2)
		{
			// << i << ": " << buffer[i + 1] << " --- " << sum << " (" << buffer[i] << ")" << endl;

			if (buffer[i + 1] != sum)
			{
				errors++;
			}

			sum += buffer[i];
		}

		cout << " errors: " << errors;
		*/

		/*
		chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();

		unsigned int * buffer = new unsigned int[_resolution * _resolution * _resolution * 2];
		_cell_info.get(_resolution * _resolution * _resolution * 2 * sizeof(unsigned int), buffer);

		unsigned int sum = 0;

		for (unsigned int i = 0; i < _resolution * _resolution * _resolution * 2; i += 2)
		{
			buffer[i + 1] = sum;
			sum += buffer[i];
		}

		_cell_info.set(_resolution * _resolution * _resolution * 2 * sizeof(unsigned int), buffer);
		delete[] buffer;

		chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();

		time += chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000.f;
		cout << " p: " << chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000.f << " ms";
		*/

	// particle sorting

		_sorting.use();

		particles.data().bind(0);
		_particle_info.bind(1);
		_cell_info.bind(2);
		_sorted_particles.bind(6);
		_sorting.set("particle_count", particles.number());

		groups = ceil(particles.number() / 1);

		glGenQueries(2, timer_queries);
		glQueryCounter(timer_queries[0], GL_TIMESTAMP);

		_sorting.execute(groups);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glQueryCounter(timer_queries[1], GL_TIMESTAMP);
		glGetQueryObjectui64v(timer_queries[0], GL_QUERY_RESULT, &time_0);
		glGetQueryObjectui64v(timer_queries[1], GL_QUERY_RESULT, &time_1);

		time += double(time_1 - time_0) / 1000000;
		cout << " sorting: " << double(time_1 - time_0) / 1000000 << " ms";
}

void hashgrid::get(unsigned int & resolution, storage * & info, storage * & particles)
{
	resolution = _resolution;
	info = &_cell_info;
	particles = &_sorted_particles;
}