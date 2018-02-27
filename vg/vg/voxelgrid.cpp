#include "voxelgrid.hpp"


#include <fstream>
#include <iostream>
#include <chrono>


voxelgrid::voxelgrid(unsigned int resolution)
{
	_resolution = resolution;

	_elements = 0;
	for (unsigned int level = resolution; level >= 1; level /= 2) _elements += level * level * level;

	_voxel_data.set(sizeof(float) * _elements);
	_scatter.source("../vg/scattering.glcs");
	_gather.source("../vg/scattering2.glcs");
	//_gather.source("../vg/gathering.glcs");
	_mipmap.source("../vg/mipmap.glcs");

	_scatter3DTexture.source("../vg/scattering3DTexture.glvs", "../vg/scattering3DTexture.glgs", "../vg/scattering3DTexture.glfs");

	scattering_iteration = 0;
	gathering_iteration = 0;
	scattering_sum = 0;
	gathering_sum = 0;

	for (unsigned int i = 0; i < 10; i++)
	{
		scattering_time[i] = 0;
		gathering_time[i] = 0;
	}
}

voxelgrid::~voxelgrid()
{

}

void voxelgrid::save(string path)
{
	unsigned int * buffer = new unsigned int[_elements];
	_voxel_data.get(sizeof(unsigned int) * _elements, buffer);

	unsigned int level_start = 0;

	for (unsigned int level = _resolution; level >= 1; level /= 2)
	{
		ofstream file(to_string(level) + path, ios::binary);
		file.write((char *)(buffer + level_start), sizeof(unsigned int) * level * level * level);
		file.close();

		level_start += level * level * level;
	}

	delete[] buffer;
}

void voxelgrid::scatter(particles & particles)
{
	auto start = chrono::high_resolution_clock::now();

	// _voxel_data.clear();

	_scatter.use();

	float particle_size = particles.size();
	float voxel_size = 1.f / _resolution;
	float voxel_size_squared = voxel_size * voxel_size;
	float voxel_radius = sqrt(voxel_size_squared + voxel_size_squared + voxel_size_squared) / 2;
	float voxel_inside_distance = particle_size - voxel_radius;
	float particle_inside_distance = voxel_radius - particle_size;
	float outside_distance = particle_size + voxel_radius;
	_scatter.set("voxel_size", 1.f / _resolution);
	_scatter.set("particle_size", particles.size());
	_scatter.set("voxel_count", _resolution);
	_scatter.set("particle_count", particles.number());
	_scatter.set("particle_size_squared", particle_size * particle_size);
	_scatter.set("voxel_inside_distance_squared", voxel_inside_distance * voxel_inside_distance);
	_scatter.set("particle_inside_distance_squared", particle_inside_distance * particle_inside_distance);
	_scatter.set("outside_distance_squared", outside_distance * outside_distance);

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

	auto end = chrono::high_resolution_clock::now();

	scattering_sum -= scattering_time[scattering_iteration];
	scattering_time[scattering_iteration] = chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000;
	scattering_sum += scattering_time[scattering_iteration];

	cout << "avg scattering: " << scattering_sum / 10 << " ms (" << float(time_1 - time_0) / 1000000 << " ms)" << endl;

	scattering_iteration = (scattering_iteration + 1) % 10;
}

void voxelgrid::scatter3DTexture(particles & p)
{
	GLuint vao;
	GLuint texture;
	GLuint framebuffer;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_3D, texture);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, _resolution, _resolution, _resolution, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0);

	auto start = chrono::high_resolution_clock::now();

	// Set Viewport size for one slice
	glViewport(0, 0, _resolution, _resolution);

	_scatter3DTexture.use();

	_scatter3DTexture.set("inGlobalRadius", p.size());

	GLuint timer_queries[2];
	glGenQueries(2, timer_queries);
	glQueryCounter(timer_queries[0], GL_TIMESTAMP);

	// Splat all points into the volume
	p.data().bind(0);
	_scatter3DTexture.execute(GL_POINTS, 0, p.number());

	glQueryCounter(timer_queries[1], GL_TIMESTAMP);
	GLuint64 time_0, time_1;
	glGetQueryObjectui64v(timer_queries[0], GL_QUERY_RESULT, &time_0);
	glGetQueryObjectui64v(timer_queries[1], GL_QUERY_RESULT, &time_1);

	auto end = chrono::high_resolution_clock::now();

	cerr << "3D Texture scattering: " << chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000 << " ms (" << (time_1 - time_0) / 1000000 << ")" << endl;

	//glDisable(GL_TEXTURE_3D);
	glDisable(GL_BLEND);

	glDeleteVertexArrays(1, &vao);
	glDeleteTextures(1, &texture);
	glDeleteFramebuffers(1, &framebuffer);
}


void voxelgrid::gather(hashgrid & hashgrid)
{
	_voxel_data.clear();

	auto start = chrono::high_resolution_clock::now();

	_gather.use();

	hashgrid.get_cell_info().bind(2);
	hashgrid.get_particle_data().bind(6);
	_voxel_data.bind(7);
	float voxel_size = 1.f / _resolution;
	float voxel_size_squared = voxel_size * voxel_size;
	float voxel_radius = sqrt(voxel_size_squared + voxel_size_squared + voxel_size_squared) / 2;
	_gather.set("voxel_size", voxel_size);
	_gather.set("voxel_count", _resolution);
	_gather.set("cell_count", hashgrid.get_resolution());
	float particle_size = 0.003f;
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
	
	//_gather.execute(groups, groups, groups * 2);
	_gather.execute(_resolution / 8, _resolution / 8, _resolution / 8);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	
	GLuint64 time_0, time_1;
	glQueryCounter(timer_queries[1], GL_TIMESTAMP);
	glGetQueryObjectui64v(timer_queries[0], GL_QUERY_RESULT, &time_0);
	glGetQueryObjectui64v(timer_queries[1], GL_QUERY_RESULT, &time_1);

	auto end = chrono::high_resolution_clock::now();

	gathering_sum -= gathering_time[gathering_iteration];
	gathering_time[gathering_iteration] = chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000.f;
	gathering_sum += gathering_time[gathering_iteration];

	cout << "avg gathering: " << gathering_sum / 10 << " ms (" << float(time_1 - time_0) / 1000000 << " ms)" << endl;

	gathering_iteration = (gathering_iteration + 1) % 10;
}

void voxelgrid::mipmap()
{
	_mipmap.use();

	_voxel_data.bind(7);

	unsigned int level_start = 0;

	GLuint timer_queries[2];
	glGenQueries(2, timer_queries);
	glQueryCounter(timer_queries[0], GL_TIMESTAMP);

	for (unsigned int level = _resolution; level > 1; level /= 2)
	{
		_mipmap.set("source_level_start", level_start);
		_mipmap.set("source_level_resolution", level);
		
		level_start += level * level * level;

		_mipmap.set("destination_level_start", level_start);
		_mipmap.set("destination_level_resolution", level / 2);

		unsigned int groups = ceil(level / 4.f);

		_mipmap.execute(groups, groups, groups * 2);
		
		// cout << "destination_level_start: " << level_start << endl;
	};

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	GLuint64 start, end;
	glQueryCounter(timer_queries[1], GL_TIMESTAMP);
	glGetQueryObjectui64v(timer_queries[0], GL_QUERY_RESULT, &start);
	glGetQueryObjectui64v(timer_queries[1], GL_QUERY_RESULT, &end);

	cout << " mipmap: " << double(end - start) / 1000000 << " ms" << endl;
}