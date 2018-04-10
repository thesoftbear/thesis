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
	_scatter_unsorted.source("../vg/scattering_unsorted.glcs");
	_scatter_sorted.source("../vg/scattering_sorted2.glcs");
	_gather.source("../vg/gathering.glcs");
	_mipmap.source("../vg/mipmap.glcs");

	_scatter_texture.source("../vg/scattering_texture.glvs", "../vg/scattering_texture.glgs", "../vg/scattering_texture.glfs");

	_scatter_texture2.source("../vg/scattering_sorted3.glcs");

	glGenTextures(1, &_voxel_texture);
	glBindTexture(GL_TEXTURE_3D, _voxel_texture);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, _resolution, _resolution, _resolution, 0, GL_RED, GL_FLOAT, NULL);
	glGenerateMipmap(GL_TEXTURE_3D);
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
	_voxel_data.clear();

	auto start = chrono::high_resolution_clock::now();

	_scatter_unsorted.use();

	float particle_size = particles.size();
	float voxel_size = 1.f / _resolution;
	float voxel_size_squared = voxel_size * voxel_size;
	float voxel_radius = sqrt(voxel_size_squared + voxel_size_squared + voxel_size_squared) / 2;
	float voxel_inside_distance = particle_size - voxel_radius;
	float particle_inside_distance = voxel_radius - particle_size;
	float outside_distance = particle_size + voxel_radius;
	_scatter_unsorted.set("voxel_size", 1.f / _resolution);
	_scatter_unsorted.set("particle_size", particles.size());
	_scatter_unsorted.set("voxel_count", _resolution);
	_scatter_unsorted.set("particle_count", particles.number());
	_scatter_unsorted.set("particle_size_squared", particle_size * particle_size);
	_scatter_unsorted.set("voxel_inside_distance_squared", voxel_inside_distance * voxel_inside_distance);
	_scatter_unsorted.set("particle_inside_distance_squared", particle_inside_distance * particle_inside_distance);
	_scatter_unsorted.set("outside_distance_squared", outside_distance * outside_distance);

	particles.data().bind(0);
	_voxel_data.bind(1);

	GLuint timer_queries[2];
	glGenQueries(2, timer_queries);
	glQueryCounter(timer_queries[0], GL_TIMESTAMP);

	unsigned int groups = ceil(particles.number() / 32.f);
	_scatter_unsorted.execute(groups);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glQueryCounter(timer_queries[1], GL_TIMESTAMP);
	GLuint64 time_0, time_1;
	glGetQueryObjectui64v(timer_queries[0], GL_QUERY_RESULT, &time_0);
	glGetQueryObjectui64v(timer_queries[1], GL_QUERY_RESULT, &time_1);

	auto end = chrono::high_resolution_clock::now();

	cout << "scattering unsorted: " << float(time_1 - time_0) / 1000000 << " ms (" << chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000 << " ms)" << endl;
}

void voxelgrid::scatter(hashgrid & hashgrid)
{
	_voxel_data.clear();

	auto start = chrono::high_resolution_clock::now();

	_scatter_sorted.use();

	hashgrid.get_particle_data().bind(0);
	_voxel_data.bind(1);

	float voxel_size = 1.f / _resolution;
	float voxel_size_squared = voxel_size * voxel_size;
	float voxel_radius = sqrt(voxel_size_squared + voxel_size_squared + voxel_size_squared) / 2;
	_scatter_sorted.set("voxel_size", voxel_size);
	_scatter_sorted.set("voxel_count", _resolution);
	float particle_size = hashgrid.get_particle_size();
	unsigned int particle_count = hashgrid.get_particle_number();
	std::cout << "parts: " << particle_count << std::endl;
	_scatter_sorted.set("particle_count", particle_count);
	_scatter_sorted.set("particle_size", particle_size);
	float voxel_inside_distance = particle_size - voxel_radius;
	_scatter_sorted.set("voxel_inside_distance_squared", voxel_inside_distance * voxel_inside_distance);
	float particle_inside_distance = voxel_radius - particle_size;
	_scatter_sorted.set("particle_inside_distance_squared", particle_inside_distance * particle_inside_distance);
	float outside_distance = particle_size + voxel_radius;
	_scatter_sorted.set("outside_distance_squared", outside_distance * outside_distance);
	_scatter_sorted.set("particle_size_squared", particle_size * particle_size);

	unsigned int groups = ceil(particle_count / 32.f);
	
	GLuint timer_queries[2];
	glGenQueries(2, timer_queries);
	glQueryCounter(timer_queries[0], GL_TIMESTAMP);

	_scatter_sorted.execute(groups, 1, 1);
	
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	GLuint64 time_0, time_1;
	glQueryCounter(timer_queries[1], GL_TIMESTAMP);
	glGetQueryObjectui64v(timer_queries[0], GL_QUERY_RESULT, &time_0);
	glGetQueryObjectui64v(timer_queries[1], GL_QUERY_RESULT, &time_1);

	auto end = chrono::high_resolution_clock::now();

	cout << "scattering sorted: " << float(time_1 - time_0) / 1000000 << " ms (" << chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000 << " ms)" << endl;
}

void voxelgrid::scatterTexture(particles & p)
{
	GLint last_framebuffer;

	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &last_framebuffer);

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
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, _resolution, _resolution, _resolution, 0, GL_RED, GL_FLOAT, NULL);

	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	auto start = chrono::high_resolution_clock::now();

	glViewport(0, 0, _resolution, _resolution);

	_scatter_texture.use();

	_scatter_texture.set("particle_radius", p.size());

	GLuint timer_queries[2];
	glGenQueries(2, timer_queries);
	glQueryCounter(timer_queries[0], GL_TIMESTAMP);

	// Splat all points into the volume
	p.data().bind(0);
	_scatter_texture.execute(GL_POINTS, 0, p.number());

	glQueryCounter(timer_queries[1], GL_TIMESTAMP);
	GLuint64 time_0, time_1;
	glGetQueryObjectui64v(timer_queries[0], GL_QUERY_RESULT, &time_0);
	glGetQueryObjectui64v(timer_queries[1], GL_QUERY_RESULT, &time_1);

	auto end = chrono::high_resolution_clock::now();

	cerr << "scattering texture: " << chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000 << " ms (" << (time_1 - time_0) / 1000000 << ")" << endl;

	// save 3D Texture to file

	/*
	float * buffer = new float[_resolution * _resolution * _resolution];
	glGetTexImage(GL_TEXTURE_3D, 0, GL_RED, GL_FLOAT, buffer);

	ofstream file("scatter_rasterizer.raw", ios::binary);
	file.write((char *)buffer, sizeof(float) * _resolution * _resolution * _resolution);
	file.close();

	delete[] buffer;
	*/
	glBindFramebuffer(GL_FRAMEBUFFER, last_framebuffer);
	glDisable(GL_BLEND);

	glDeleteVertexArrays(1, &vao);
	glDeleteTextures(1, &texture);
	glDeleteFramebuffers(1, &framebuffer);
}

void voxelgrid::scatterTexture(hashgrid & hashgrid)
{
	GLint last_framebuffer;

	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &last_framebuffer);

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
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, _resolution, _resolution, _resolution, 0, GL_RED, GL_FLOAT, NULL);

	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	auto start = chrono::high_resolution_clock::now();

	glViewport(0, 0, _resolution, _resolution);

	_scatter_texture.use();

	_scatter_texture.set("particle_radius", hashgrid.get_particle_size());

	GLuint timer_queries[2];
	glGenQueries(2, timer_queries);
	glQueryCounter(timer_queries[0], GL_TIMESTAMP);

	// Splat all points into the volume
	hashgrid.get_particle_data().bind(0);
	_scatter_texture.execute(GL_POINTS, 0, hashgrid.get_particle_number());

	glQueryCounter(timer_queries[1], GL_TIMESTAMP);
	GLuint64 time_0, time_1;
	glGetQueryObjectui64v(timer_queries[0], GL_QUERY_RESULT, &time_0);
	glGetQueryObjectui64v(timer_queries[1], GL_QUERY_RESULT, &time_1);

	auto end = chrono::high_resolution_clock::now();

	cerr << "scattering texture (sorted): " << chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000 << " ms (" << (time_1 - time_0) / 1000000 << ")" << endl;

	// save 3D Texture to file

	/*
	float * buffer = new float[_resolution * _resolution * _resolution];
	glGetTexImage(GL_TEXTURE_3D, 0, GL_RED, GL_FLOAT, buffer);

	ofstream file("scatter_rasterizer.raw", ios::binary);
	file.write((char *)buffer, sizeof(float) * _resolution * _resolution * _resolution);
	file.close();

	delete[] buffer;
	*/
	glBindFramebuffer(GL_FRAMEBUFFER, last_framebuffer);
	glDisable(GL_BLEND);

	glDeleteVertexArrays(1, &vao);
	glDeleteTextures(1, &texture);
	glDeleteFramebuffers(1, &framebuffer);
}

void voxelgrid::scatterTexture2(hashgrid & hashgrid)
{
	auto start = chrono::high_resolution_clock::now();

	_scatter_texture2.use();

	_scatter_texture2.set("voxel_texture", 0);
	glBindImageTexture(0, _voxel_texture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32F);

	hashgrid.get_particle_data().bind(0);

	float voxel_size = 1.f / _resolution;
	float voxel_size_squared = voxel_size * voxel_size;
	float voxel_radius = sqrt(voxel_size_squared + voxel_size_squared + voxel_size_squared) / 2;
	_scatter_texture2.set("voxel_size", voxel_size);
	_scatter_texture2.set("voxel_count", _resolution);

	float particle_size = hashgrid.get_particle_size();
	unsigned int particle_count = hashgrid.get_particle_number();
	_scatter_texture2.set("particle_count", particle_count);
	_scatter_texture2.set("particle_size", particle_size);
	
	float voxel_inside_distance = particle_size - voxel_radius;
	_scatter_texture2.set("voxel_inside_distance_squared", voxel_inside_distance * voxel_inside_distance);
	float particle_inside_distance = voxel_radius - particle_size;
	_scatter_texture2.set("particle_inside_distance_squared", particle_inside_distance * particle_inside_distance);
	float outside_distance = particle_size + voxel_radius;
	_scatter_texture2.set("outside_distance_squared", outside_distance * outside_distance);
	_scatter_texture2.set("particle_size_squared", particle_size * particle_size);

	unsigned int groups = ceil(particle_count / 32.f);

	GLuint timer_queries[2];
	glGenQueries(2, timer_queries);
	glQueryCounter(timer_queries[0], GL_TIMESTAMP);

	_scatter_texture2.execute(groups, 1, 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	GLuint64 time_0, time_1;
	glQueryCounter(timer_queries[1], GL_TIMESTAMP);
	glGetQueryObjectui64v(timer_queries[0], GL_QUERY_RESULT, &time_0);
	glGetQueryObjectui64v(timer_queries[1], GL_QUERY_RESULT, &time_1);

	auto end = chrono::high_resolution_clock::now();

	cout << "scattering texture 2: " << float(time_1 - time_0) / 1000000 << " ms (" << chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000 << " ms)" << endl;

	glBindTexture(GL_TEXTURE_3D, _voxel_texture);
	glGenerateMipmap(GL_TEXTURE_3D);
	
	unsigned int level = 0;

	for (unsigned int resolution = 256; resolution > 0; resolution /= 2)
	{
		float * pixels = new float[resolution * resolution * resolution];

		glGetTexImage(GL_TEXTURE_3D, level, GL_RED, GL_FLOAT, pixels);
		
		bool valid = 0;

		for (unsigned int i = 0; i < resolution * resolution * resolution; i++)
		{
			if (pixels[i] != 0.f)
			{
				valid = true;
				break;
			}
		}
		
		cout << "level " << level << " is " << (valid ? "valid" : "empty") << endl;

		delete[] pixels;
		
		level++;
	}

	glBindTexture(GL_TEXTURE_3D, 0);
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
	float particle_size = hashgrid.get_particle_size();
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
	
	_gather.execute(groups, groups, groups * 2);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	
	GLuint64 time_0, time_1;
	glQueryCounter(timer_queries[1], GL_TIMESTAMP);
	glGetQueryObjectui64v(timer_queries[0], GL_QUERY_RESULT, &time_0);
	glGetQueryObjectui64v(timer_queries[1], GL_QUERY_RESULT, &time_1);

	auto end = chrono::high_resolution_clock::now();

	cout << "gathering: " << float(time_1 - time_0) / 1000000 << " ms (" << chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000 << " ms)" << endl;
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
void voxelgrid::mipmapTexture()
{
	glBindTexture(GL_TEXTURE_3D, _voxel_texture);
	glGenerateMipmap(GL_TEXTURE_3D);
	glBindTexture(GL_TEXTURE_3D, 0);
}

unsigned int voxelgrid::get_resolution()
{
	return _resolution;
}

GLuint voxelgrid::get_texture()
{
	return _voxel_texture;
}

storage & voxelgrid::get_data()
{
	return _voxel_data;
}
