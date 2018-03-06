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
	auto start = chrono::high_resolution_clock::now();

	// allocate storage according to number of particles

		if (_sorted_particles.size() != particles.data().size())
		{
			_sorted_particles.set(particles.data().size(), nullptr, GL_DYNAMIC_COPY);

			_particle_info.set(particles.number() * 2 * sizeof(unsigned int), nullptr, GL_DYNAMIC_COPY);
			_particle_info.clear();
		}

		particle_size = particles.size();
		particle_number = particles.number();

	// clear old cell info

		_cell_info.clear();
		
	// spatial hashing

		_hashing.use();

		_hashing.set("particle_count", particles.number());
		_hashing.set("cell_count", _resolution);
		particles.data().bind(0);
		_particle_info.bind(1);
		_cell_info.bind(2);

		unsigned int groups = ceil(particles.number() / 32.f);

		GLuint timer_queries[2];
		glGenQueries(2, timer_queries);
		glQueryCounter(timer_queries[0], GL_TIMESTAMP);

		_hashing.execute(groups);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glQueryCounter(timer_queries[1], GL_TIMESTAMP);
		GLuint64 time_0, time_1;
		glGetQueryObjectui64v(timer_queries[0], GL_QUERY_RESULT, &time_0);
		glGetQueryObjectui64v(timer_queries[1], GL_QUERY_RESULT, &time_1);

	float hashing_time = float(time_1 - time_0) / 1000000;

	// verify hashing

		/*
		struct vec4
		{
			float x, y, z, a;
		};

		vec4 * particle_data = new vec4[particles.number()];
		particles.data().get(particles.number() * sizeof(vec4), particle_data);

		struct uvec2
		{
			unsigned int x, y;
		};

		uvec2 * gpu_particle_info = new uvec2[particles.number()];
		_particle_info.get(sizeof(uvec2) * particles.number(), gpu_particle_info);

		uvec2 * gpu_cell_info = new uvec2[_resolution * _resolution * _resolution];
		_cell_info.get(sizeof(uvec2) * _resolution * _resolution * _resolution, gpu_cell_info);
		
		uvec2 * cpu_cell_info = new uvec2[_resolution * _resolution * _resolution];

		unsigned int particle_error = 0;

		for (unsigned int i = 0; i < particles.number(); i++)
		{
			vec4 & position = particle_data[i];

			unsigned int cell_x = position.x * _resolution;
			unsigned int cell_y = position.y * _resolution;
			unsigned int cell_z = position.z * _resolution;

			unsigned int cell_index = (cell_z * _resolution + cell_y) * _resolution + cell_x;

			if (gpu_particle_info[i].x != cell_index) particle_error++;

			cpu_cell_info[cell_index].x++; 
		}
		
		cout << " particle_error: " << particle_error;

		unsigned int cell_error = 0;

		for (unsigned int i = 0; i < _resolution * _resolution * _resolution; i++)
		{
			if (gpu_cell_info[i].x != cpu_cell_info[i].x) cell_error++;
		}

		cout << " cell_error: " << cell_error;

		*/

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

	float prefix_time = float(time_1 - time_0) / 1000000;
	
	// verify prefix sum
		
		/*
		_cell_info.get(_resolution * _resolution * _resolution * sizeof(uvec2), gpu_cell_info);

		unsigned int gpu_sum = 0;
		unsigned int cpu_sum = 0;
		unsigned int errors = 0;

		for (unsigned int i = 0; i < _resolution * _resolution * _resolution ; i++)
		{
			if (gpu_cell_info[i].y != gpu_sum || gpu_cell_info[i].y != cpu_sum)
			{
				errors++;
			}

			gpu_sum += gpu_cell_info[i].x;
			cpu_sum += cpu_cell_info[i].x;
		}

		cout << " errors: " << errors;
		
		*/

	// particle sorting

		_sorting.use();

		particles.data().bind(0);
		_particle_info.bind(1);
		_cell_info.bind(2);
		_sorted_particles.bind(6);
		_sorting.set("particle_count", particles.number());

		groups = ceil(particles.number() / 32.f);

		glGenQueries(2, timer_queries);
		glQueryCounter(timer_queries[0], GL_TIMESTAMP);

		_sorting.execute(groups);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glQueryCounter(timer_queries[1], GL_TIMESTAMP);
		glGetQueryObjectui64v(timer_queries[0], GL_QUERY_RESULT, &time_0);
		glGetQueryObjectui64v(timer_queries[1], GL_QUERY_RESULT, &time_1);

	float sorting_time = float(time_1 - time_0) / 1000000;

	// verify sorting

		/*
		vec4 * gpu_sorted_particles = new vec4[particles.number()];
		_sorted_particles.get(particles.number() * sizeof(vec4), gpu_sorted_particles);

		unsigned int sorting_error = 0;

		for (unsigned int i = 0; i < particles.number(); i++)
		{	
			vec4 particle = particle_data[i];
			uvec2 particle_info = gpu_particle_info[i];

			unsigned int cpu_index = gpu_cell_info[particle_info.x].y + particle_info.y;

			vec4 gpu_particle = gpu_sorted_particles[cpu_index];

			if (gpu_particle.x != particle.x || gpu_particle.y != particle.y || gpu_particle.z != particle.z) sorting_error++;
		}

		cout << " error: " << sorting_error;
		*/

	auto end = chrono::high_resolution_clock::now();

	cout << "hashgrid: " << chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000 << " ms (hashing: "
		<< hashing_time << " ms, prefix: " << prefix_time << " ms, sorting: " << sorting_time << " ms)" << endl;
}

storage & hashgrid::get_cell_info()
{
	return _cell_info;
}

storage & hashgrid::get_particle_data()
{
	return _sorted_particles;
}

unsigned int hashgrid::get_resolution()
{
	return _resolution;
}

unsigned int hashgrid::get_particle_number()
{
	return particle_number;
}

float hashgrid::get_particle_size()
{
	return particle_size;
}