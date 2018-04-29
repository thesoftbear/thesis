#include "voxel_octree.hpp"
#include "clock.hpp"

#include <fstream>
#include <iostream>
#include <chrono>

#include "gtc\constants.hpp"
#include <gtc\integer.hpp>

voxel_octree::voxel_octree()
{
	scatter_shader.source("../vg/scatter.glcs");
	gather_shader.source("../vg/gather.glcs");
	mipmap_shader.source("../vg/mipmap.glcs");
	clamp_shader.source("../vg/clamp.glcs");

	scatter_rasterizer_shader.source("../vg/scatter_rasterizer.glvs", "../vg/scatter_rasterizer.glgs", "../vg/scatter_rasterizer.glfs");

	glGenTextures(1, &voxel_texture);
	glBindTexture(GL_TEXTURE_3D, voxel_texture);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glBindTexture(GL_TEXTURE_3D, 0);

	set_resolution(32);
}

voxel_octree::~voxel_octree()
{
	glDeleteTextures(1, &voxel_texture);
}

void voxel_octree::set_resolution(unsigned int resolution)
{
	this->resolution = resolution;

	glBindTexture(GL_TEXTURE_3D, voxel_texture);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, resolution, resolution, resolution, 0, GL_RED, GL_FLOAT, NULL);
	glGenerateMipmap(GL_TEXTURE_3D);
	glBindTexture(GL_TEXTURE_3D, 0);
}

void voxel_octree::save_level(string path, unsigned int level)
{
	level = max(level, glm::log2(level));

	unsigned int level_resolution = resolution / (unsigned int)pow(2, level);

	float * buffer = new float[level_resolution * level_resolution * level_resolution];
	glGetTexImage(GL_TEXTURE_3D, level, GL_RED, GL_FLOAT, buffer);

	ofstream file("voxel_" + to_string(level) + ".raw", ios::binary);
	file.write((char *)buffer, sizeof(float) * level_resolution * level_resolution * level_resolution);
	file.close();

	delete[] buffer;
}

void voxel_octree::clear_texture()
{
	/*
		glTexSubImage2D();
		glClearTexImage();
	*/ 
}

void voxel_octree::scatter_particles(particle_data & particles)
{
	float elapsed_time = scatter_particles(particles.get_count(), particles.get_radius(), particles.get_data());

	std::cout << "scattering unsorted: " << elapsed_time << " ms" << endl;
}

void voxel_octree::scatter_particles(space_partitioning_grid & grid)
{
	float elapsed_time = scatter_particles(grid.get_particle_count(), grid.get_particle_radius(), grid.get_particle_storage());

	std::cout << "scattering coalesced: " << elapsed_time << " ms" << endl;
}


float voxel_octree::scatter_particles(unsigned int count, float radius, storage & positions)
{
	// setup gpu clock

	thesis::clock clock;
	clock.start_timer();

	// start with an empty octree

	clear_texture();

	// activate scattering shader

	scatter_shader.use();

	// setup particle data

	positions.bind(0);

	scatter_shader.set("particle_count", count);
	scatter_shader.set("particle_radius", radius);
	scatter_shader.set("particle_radius_squared", pow(radius, 2));

	// setup voxel octree

	scatter_shader.set("voxel_texture", 0);
	glBindImageTexture(0, voxel_texture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32F);
	
	scatter_shader.set("voxel_resolution", resolution);
	float voxel_size = 1.f / resolution;
	scatter_shader.set("voxel_size", voxel_size);

	scatter_shader.set("volume_ratio", ((4.f / 3.f) * glm::pi<float>() * pow(radius, 3)) / pow(voxel_size, 3)); // particle volume / voxel volume

	float voxel_radius = sqrt(pow(voxel_size, 2) * 3) / 2;
	scatter_shader.set("voxel_inside_particle_distance_squared", pow(radius - voxel_radius, 2));
	scatter_shader.set("particle_inside_voxel_distance_squared", pow(voxel_radius - radius, 2));
	scatter_shader.set("no_intersection_distance_squared", pow(radius + voxel_radius, 2));

	// start scattering shader

	unsigned int work_groups = (unsigned int)ceil(count / 32.f);
	scatter_shader.execute(work_groups);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	// mipmap finest level density

	clamp_texture();
	create_mipmap();

	// return elapsed time

	clock.stop_timer();
	return clock.get_time();
}

void voxel_octree::scatter_particles_rasterizer(particle_data & particles)
{
	float elapsed_time = scatter_particles_rasterizer(particles.get_count(), particles.get_radius(), particles.get_data());

	std::cout << "scattering rasterizer unsorted: " << elapsed_time << " ms" << endl;
}

void voxel_octree::scatter_particles_rasterizer(space_partitioning_grid & grid)
{
	float elapsed_time = scatter_particles_rasterizer(grid.get_particle_count(), grid.get_particle_radius(), grid.get_particle_storage());

	std::cout << "scattering rasterizer sorted: " << elapsed_time << " ms" << endl;
}

float voxel_octree::scatter_particles_rasterizer(unsigned int count, float radius, storage & positions)
{
	thesis::clock clock;
	clock.start_timer();

	GLint last_framebuffer;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &last_framebuffer);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	GLuint framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, voxel_texture, 0);
	
	glClear(GL_COLOR_BUFFER_BIT);

	auto start = chrono::high_resolution_clock::now();

	glViewport(0, 0, resolution, resolution);

	scatter_rasterizer_shader.use();

	scatter_rasterizer_shader.set("particle_radius", radius);

	GLuint timer_queries[2];
	glGenQueries(2, timer_queries);
	glQueryCounter(timer_queries[0], GL_TIMESTAMP);

	// Splat all points into the volume
	positions.bind(0);
	scatter_rasterizer_shader.execute(GL_POINTS, 0, count);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, last_framebuffer);
	glDeleteFramebuffers(1, &framebuffer);
	glDisable(GL_BLEND);
	glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glDeleteVertexArrays(1, &vao);

	clock.stop_timer();

	return clock.get_time();
}

void voxel_octree::gather_particles(space_partitioning_grid & grid)
{
	float elapsed_time = gather_particles(grid.get_particle_count(), grid.get_particle_radius(), grid.get_particle_storage(), grid.get_resolution(), grid.get_cell_info());

	std::cout << "gathering partitioning: " << elapsed_time << " ms" << std::endl;
}

void voxel_octree::gather_particles(particle_data & particles)
{
	uvec2 cell_info_data = uvec2(particles.get_count(), 0);

	storage single_cell_info;
	single_cell_info.set(sizeof(cell_info_data), &cell_info_data, GL_STATIC_READ);

	float elapsed_time = gather_particles(particles.get_count(), particles.get_radius(), particles.get_data(), 1, single_cell_info);

	std::cout << "gathering direct: " << elapsed_time << " ms" << std::endl;
}

float voxel_octree::gather_particles(unsigned int count, float radius, storage & positions, unsigned int grid_resolution, storage & cell_info)
{
	thesis::clock clock;
	clock.start_timer();

	gather_shader.use();

	// setup grid data

	cell_info.bind(2);
	positions.bind(6);

	gather_shader.set("grid_resolution", grid_resolution);
	gather_shader.set("particle_radius", radius);
	gather_shader.set("particle_radius_squared", pow(radius, 2));

	// setup voxel octree data

	gather_shader.set("voxel_texture", 0);
	glBindImageTexture(0, voxel_texture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32F);

	gather_shader.set("voxel_resolution", resolution);
	float voxel_size = 1.f / resolution;
	gather_shader.set("voxel_size", voxel_size);
	gather_shader.set("volume_ratio", ((4.f / 3.f) * glm::pi<float>() * pow(radius, 3)) / pow(voxel_size, 3)); // particle volume / voxel volume

	float voxel_radius = sqrt(pow(voxel_size, 2) * 3) / 2;
	gather_shader.set("voxel_inside_particle_distance_squared", pow(radius - voxel_radius, 2));
	gather_shader.set("particle_inside_voxel_distance_squared", pow(voxel_radius - radius, 2));
	gather_shader.set("no_intersection_distance_squared", pow(radius + voxel_radius, 2));

	unsigned int groups = (unsigned int)ceil(resolution / 4.f);

	gather_shader.execute(groups, groups, groups * 2);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	clamp_texture();
	create_mipmap();

	clock.stop_timer();

	return clock.get_time();
}

void voxel_octree::create_mipmap()
{
	// propagate through hierarchy

	mipmap_shader.use();

	mipmap_shader.set("input_texture", 0);
	mipmap_shader.set("output_texture", 1);

	for (unsigned int level = 0; level < glm::log2(resolution); level++)
	{	
		glBindImageTexture(0, voxel_texture, level, GL_TRUE, 0, GL_READ_ONLY, GL_R32F);
		glBindImageTexture(1, voxel_texture, level + 1, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);

		unsigned int output_resolution = resolution / pow(2, level + 1);
		mipmap_shader.set("output_resolution", output_resolution);

		uvec3 work_groups;
		work_groups.x = ceil(output_resolution / 4.f);
		work_groups.y = work_groups.x;
		work_groups.z = work_groups.x * 2;

		mipmap_shader.execute(work_groups.x, work_groups.y, work_groups.z);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}
}

void voxel_octree::clamp_texture()
{
	// clamp density values

	clamp_shader.use();

	clamp_shader.set("clamp_texture", 0);
	glBindImageTexture(0, voxel_texture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32F);

	uvec3 work_groups;
	work_groups.x = resolution / 4;
	work_groups.y = work_groups.x;
	work_groups.z = work_groups.x * 2;

	clamp_shader.execute(work_groups.x, work_groups.y, work_groups.z);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

unsigned int voxel_octree::get_resolution()
{
	return resolution;
}

GLuint voxel_octree::get_texture()
{
	return voxel_texture;
}