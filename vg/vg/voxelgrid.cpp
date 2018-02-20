#include "voxelgrid.hpp"


#include <fstream>
#include <iostream>

voxelgrid::voxelgrid(unsigned int resolution)
{
	_resolution = resolution;
	_voxel_data.set(sizeof(unsigned int) * _resolution * _resolution * _resolution);
	_scatter.source("../vg/scattering.glcs");
	_gather.source("../vg/gathering.glcs");
}

voxelgrid::~voxelgrid()
{

}

void voxelgrid::save(string path)
{
	unsigned int elements = _resolution * _resolution * _resolution;
	unsigned int * buffer = new unsigned int[elements];
	_voxel_data.get(sizeof(unsigned int) * elements, buffer);

	unsigned int counter = 0;
	for (unsigned int i = 0; i < elements; i++) if (buffer[i] != 0) counter++;
	cout << "voxels != 0: " << counter << " (" << buffer[100] << ")" << endl;

	ofstream file(path, ios::binary);
	file.write((char *)buffer, sizeof(unsigned int) * elements);
	file.close();

	delete[] buffer;
}

void voxelgrid::scatter(particles & particles)
{
	std::cout << particles.size() << endl;

	_voxel_data.clear();

	_scatter.use();

	_scatter.set("cell_size", 1.f / _resolution);
	_scatter.set("particle_size", particles.size());
	_scatter.set("cell_count", _resolution);
	_scatter.set("particle_count", particles.number());

	particles.data().bind(0);
	_voxel_data.bind(1);

	GLuint timer_queries[2];
	glGenQueries(2, timer_queries);
	glQueryCounter(timer_queries[0], GL_TIMESTAMP);

	unsigned int groups = ceil(particles.number() / 32.f);
	_scatter.execute(groups);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glQueryCounter(timer_queries[1], GL_TIMESTAMP);
	GLuint64 time_0, time_1;
	glGetQueryObjectui64v(timer_queries[0], GL_QUERY_RESULT, &time_0);
	glGetQueryObjectui64v(timer_queries[1], GL_QUERY_RESULT, &time_1);

	cout << "[scattering] " << double(time_1 - time_0) / 1000000 << " ms" << endl;

	// save("scattering_v" + to_string(_resolution) + ".raw");
}

void voxelgrid::gather(hashgrid & hashgrid)
{
	_voxel_data.clear();

	_gather.use();

	unsigned int hashgrid_resolution;
	storage * cell_info;
	storage * sorted_particles;

	hashgrid.get(hashgrid_resolution, cell_info, sorted_particles);

	cell_info->bind(2);
	sorted_particles->bind(6);
	_voxel_data.bind(7);
	float voxel_size = 1.f / _resolution;
	float voxel_size_squared = voxel_size * voxel_size;
	float voxel_radius = sqrt(voxel_size_squared + voxel_size_squared + voxel_size_squared) / 2;
	_gather.set("voxel_size", voxel_size);
	_gather.set("voxel_count", _resolution);
	_gather.set("cell_count", hashgrid_resolution);
	float particle_size = 0.005f;
	_gather.set("particle_size", particle_size);
	float voxel_inside_distance = particle_size - voxel_radius;
	_gather.set("voxel_inside_distance_squared", voxel_inside_distance * voxel_inside_distance);
	float particle_inside_distance = voxel_radius - particle_size;
	_gather.set("particle_inside_distance_squared", particle_inside_distance * particle_inside_distance);
	float outside_distance = particle_size + voxel_radius;
	_gather.set("outside_distance_squared", outside_distance * outside_distance);
	_gather.set("particle_size_squared", particle_size * particle_size);

	unsigned int groups = ceil(_resolution / 4.f);

	GLuint timer_queries[2];
	glGenQueries(2, timer_queries);
	glQueryCounter(timer_queries[0], GL_TIMESTAMP);
	
	_gather.execute(groups, groups, groups);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	
	GLuint64 start, end;
	glQueryCounter(timer_queries[1], GL_TIMESTAMP);
	glGetQueryObjectui64v(timer_queries[0], GL_QUERY_RESULT, &start);
	glGetQueryObjectui64v(timer_queries[1], GL_QUERY_RESULT, &end);

	cout << " gathering: " << double(end - start) / 1000000 << " ms" << endl;
}