#include "hashgrid.hpp"

#include <iostream>
#include <chrono>

space_partitioning_grid::space_partitioning_grid()
{
	set_resolution(32);

	hashing_shader.source("../vg/hashing.glcs");
	prefixsum_shader.source("../vg/prefixsum.glcs");
	sorting_shader.source("../vg/sorting.glcs");
}

void space_partitioning_grid::set_resolution(unsigned int resolution)
{
	if (this->resolution != resolution)
	{
		this->resolution = resolution;

		cell_info.set(resolution * resolution * resolution * 2 * sizeof(unsigned int), nullptr, GL_DYNAMIC_COPY);
		cell_info.clear();
	}
}

void space_partitioning_grid::insert_particles(particle_data & particles)
{
	auto start = chrono::high_resolution_clock::now();

	// allocate storage according to number of particles

	if (particle_storage.size() != particles.get_data().size())
	{
		particle_storage.set(particles.get_data().size(), nullptr, GL_DYNAMIC_COPY);
		particle_storage.clear();
	}

	particle_radius = particles.get_radius();
	particle_count = particles.get_count();

	// allocate particle info storage

	storage particle_info;
	particle_info.set(particles.get_count() * 2 * sizeof(unsigned int), nullptr, GL_DYNAMIC_COPY);

	// clear old cell info

	cell_info.clear();
		
	// spatial hashing

	hash_particles(particles, particle_info);

	// address calculation

	calculate_addresses();

	// particle sorting
		
	sort_particles(particles, particle_info);

	auto end = chrono::high_resolution_clock::now();

	cout << "spatial partitioning: " << chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000 << " ms" << endl;
}

void space_partitioning_grid::hash_particles(particle_data & particles, storage & particle_info)
{
	hashing_shader.use();

	hashing_shader.set("particle_count", particles.get_count());
	hashing_shader.set("grid_resolution", resolution);
	particles.get_data().bind(0);
	particle_info.bind(1);
	cell_info.bind(2);

	unsigned int groups = ceil(particles.get_count() / 32.f);

	hashing_shader.execute(groups);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void space_partitioning_grid::calculate_addresses()
{
	storage prefix_data;
	prefix_data.set(3 * sizeof(unsigned int), nullptr, GL_DYNAMIC_COPY);
	prefix_data.clear();

	prefixsum_shader.use();

	cell_info.bind(2);
	prefix_data.bind(3);

	unsigned int groups = ceil((pow(resolution, 3)) / (32 * 16.f));

	prefixsum_shader.execute(groups);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void space_partitioning_grid::sort_particles(particle_data & particles, storage & particle_info)
{
	sorting_shader.use();

	particles.get_data().bind(0);
	particle_info.bind(1);
	cell_info.bind(2);
	particle_storage.bind(6);
	sorting_shader.set("particle_count", particles.get_count());

	unsigned int groups = ceil(particles.get_count() / 32.f);

	sorting_shader.execute(groups);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void space_partitioning_grid::verify_partitioning(particle_data & particles)
{
	// reconstruct on cpu side

	struct vec4
	{
		float x, y, z, a;
	};

	struct uvec2
	{
		unsigned int x, y;
	};

	vec4 * cpu_particle_data = new vec4[particles.get_count()];
	particles.get_data().get(particles.get_count() * sizeof(vec4), cpu_particle_data);

	uvec2 * cpu_particle_info = new uvec2[particles.get_count()];
	uvec2 * cpu_cell_info = new uvec2[pow(resolution, 3)];

	// cpu particle hashing

	for (unsigned int i = 0; i < particles.get_count(); i++)
	{
		vec4 & position = cpu_particle_data[i];

		unsigned int cell_x = position.x * resolution;
		unsigned int cell_y = position.y * resolution;
		unsigned int cell_z = position.z * resolution;

		unsigned int cell_index = (cell_z * resolution + cell_y) * resolution + cell_x;

		cpu_particle_info[i].x = cell_index;
		cpu_particle_info[i].y = cpu_cell_info[cell_index].x;

		cpu_cell_info[cell_index].x++;
	}

	// cpu address calculation

	unsigned int address = 0;
	
	for (unsigned int c = 0; c < pow(resolution, 3); c++)
	{
		cpu_cell_info[c].y = address;
		address += cpu_cell_info[c].x;
	}

	// cpu particle sorting

	vec4 * cpu_sorted_particle_data = new vec4[particles.get_count()];
	
	for (unsigned int i = 0; i < particles.get_count(); i++)
	{
		uvec2 particle_info = cpu_particle_info[i];
		unsigned int index = cpu_cell_info[particle_info.x].y + particle_info.y;
		cpu_sorted_particle_data[index] = cpu_particle_data[i];
	}

	// verify gpu output

	unsigned int errors = 0;
	
	// first the particles

	vec4 * gpu_sorted_particle_data = new vec4[particles.get_count()];
	particle_storage.get(particles.get_count() * sizeof(vec4), gpu_sorted_particle_data);

	for (unsigned int c = 0; c < pow(resolution, 3); c++)
	{
		uvec2 & cpu_info = cpu_cell_info[c];

		for (unsigned int p = cpu_info.y; p < cpu_info.y + cpu_info.x; p++)
		{
			vec4 & gpu_particle = gpu_sorted_particle_data[p];

			unsigned int cell_x = gpu_particle.x * resolution;
			unsigned int cell_y = gpu_particle.y * resolution;
			unsigned int cell_z = gpu_particle.z * resolution;

			unsigned int cell_index = (cell_z * resolution + cell_y) * resolution + cell_x;

			if (cell_index != c) errors++;
		}
	}

	std::cout << "particle errors: " << errors << " / " << particles.get_count() << std::endl;

	errors = 0;

	// then the grid

	uvec2 * gpu_cell_info = new uvec2[pow(resolution, 3)];
	cell_info.get(pow(resolution, 3) * sizeof(uvec2), gpu_cell_info);

	for (unsigned int c = 0; c < pow(resolution, 3); c++)
	{
		uvec2 & cpu_info = cpu_cell_info[c];
		uvec2 & gpu_info = gpu_cell_info[c];

		if (cpu_info.x != gpu_info.x || cpu_info.y != gpu_info.y) errors++;

		if (c % 1000 == 0) std::cout << cpu_info.x << " " << cpu_info.y << std::endl;
	}

	std::cout << "cell errors: " << errors << " / " << pow(resolution, 3) << std::endl;

	delete[] cpu_particle_data;
	delete[] cpu_particle_info;
	delete[] cpu_cell_info;
	delete[] cpu_sorted_particle_data;
	delete[] gpu_sorted_particle_data;
	delete[] gpu_cell_info;
}

storage & space_partitioning_grid::get_cell_info()
{
	return cell_info;
}

storage & space_partitioning_grid::get_particle_storage()
{
	return particle_storage;
}

unsigned int space_partitioning_grid::get_resolution()
{
	return resolution;
}

unsigned int space_partitioning_grid::get_particle_count()
{
	return particle_count;
}

float space_partitioning_grid::get_particle_radius()
{
	return particle_radius;
}