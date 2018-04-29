#include "particle_data.hpp"
#include "error.hpp"

#include <random>
#include <fstream>
#include <iostream>
#include <chrono>

#define GLM_ENABLE_EXPERIMENTAL

#include "gtc\noise.hpp"

particle_data::particle_data()
{
	count = 0;
	radius = 0.f;
}

particle_data::~particle_data()
{
}

void particle_data::generate_random(unsigned int count, float radius)
{
	this->radius = radius;

	float * buffer = new float[4 * count];

	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution(0.f, 1.f);

	cout << "Generating particles ... ";

	this->count = 0;

	while (this->count < count)
	{
		float x = distribution(generator);
		float y = distribution(generator);
		float z = distribution(generator);

		// sphere test

		float xs = x - 0.5f;
		float ys = y - 0.5f;
		float zs = z - 0.5f;
		if (sqrt(xs * xs + ys * ys + zs * zs) > 0.4f) continue;

		float chance = glm::simplex(glm::vec3(x * 10, y * 10, z * 10));

		float draw = distribution(generator);

		if (draw <= chance)
		{
			float * coordinates = buffer + 4 * this->count;

			coordinates[0] = x * 0.8 + 0.1;
			coordinates[1] = y * 0.8 + 0.1;
			coordinates[2] = z * 0.8 + 0.1;

			this->count++;
		}
	}

	data.set(4 * count * sizeof(float), buffer, GL_STATIC_DRAW);

	delete[] buffer;

	cout << " complete." << endl;
}

void particle_data::generate_two(float radius, float distance)
{
	this->radius = radius;

	this->count = 2;

	float coordinates[8];

	coordinates[0] = 0.5 - radius - distance / 2;
	coordinates[1] = 0.5;
	coordinates[2] = 0.5;
	coordinates[3] = 0.5;
	coordinates[4] = 0.5 + radius + distance / 2;
	coordinates[5] = 0.5;
	coordinates[6] = 0.5;
	coordinates[7] = 0.5;

	data.set(4 * 2 * sizeof(float), coordinates, GL_STATIC_DRAW);
}

void particle_data::read_file(string path, unsigned int frame)
{
	ifstream file(path, ios::beg | ios::binary);

	#pragma pack(push, 1)
	struct
	{
		char identifier[6];
		unsigned short version;
		unsigned int frames;
		float bounding_box[6];
		float clipping_box[6];
	}
	file_header;
	#pragma pack(pop)

	file.read((char *)&file_header, sizeof(file_header));

	// cout << "version: " << file_header.version << endl;

	unsigned long long * frame_offsets = new unsigned long long[file_header.frames];
	file.read((char *)frame_offsets, sizeof(unsigned long long) * file_header.frames);

	unsigned long long file_size;
	file.read((char *)&file_size, sizeof(unsigned long long));

	// cout << "file size: " << file_size << endl;

	cout << "frames: " << file_header.frames << endl;

	// read frame 

	file.seekg(frame_offsets[frame]);
	std::cout << "frame " << frame << " / " << file_header.frames << " (" << file.tellg() << ")" << std::endl;

	delete[] frame_offsets;

	if (file_header.version >= 102)
	{
		float timestamp;
		file.read((char *)&timestamp, sizeof(float));

		cout << "timestamp: " << timestamp;
	}

	unsigned int particle_lists;
	file.read((char *)&particle_lists, sizeof(unsigned int));

	cout << "particle lists: " << particle_lists << endl;

	unsigned char vertex_type;
	file.read((char *)&vertex_type, sizeof(unsigned char));

	unsigned char color_type;
	file.read((char *)&color_type, sizeof(unsigned char));

	cout << "vertex type: " << int(vertex_type) << " color type: " << int(color_type) << endl;

	if (vertex_type == 1 || vertex_type == 3)
	{
		file.read((char *)&radius, sizeof(float));
	}
	else if (vertex_type == 2)
	{
		radius = 1.0;
	}

	cout << "radius: " << radius << endl;

	unsigned char global_color[4];
	float minimum_intensity = 0;
	float maximum_intensity = 0;

	if (color_type == 0)
	{
		file.read((char *)global_color, sizeof(unsigned char) * 4);
	}
	else if (color_type == 3)
	{
		file.read((char *)&minimum_intensity, sizeof(float));
		file.read((char *)&maximum_intensity, sizeof(float));
	}

	unsigned long long particle_count = 0;
	file.read((char *)&particle_count, sizeof(unsigned long long));
	count = particle_count;

	std::cout << "count = " << count << std::endl;

	unsigned int vertex_size;
	unsigned int color_size;

	if (vertex_type == 0) vertex_size = 0;
	if (vertex_type == 1) vertex_size = 12;
	if (vertex_type == 2) vertex_size = 16;
	if (vertex_type == 3) vertex_size = 6;

	if (color_type == 0) color_size = 0;
	if (color_type == 1) color_size = 3;
	if (color_type == 2) color_size = 4;
	if (color_type == 3) color_size = 4;

	unsigned char * particle_data = new unsigned char[count * (vertex_size + color_size)];
	file.read((char *)particle_data, count * (vertex_size + color_size));

	float * storage_data = new float[count * 4];

	float min_x = 10000000;
	float max_x = -10000000;
	float min_y = 10000000;
	float max_y = -10000000;
	float min_z = 10000000;
	float max_z = -10000000;

	if (vertex_type == 1 || vertex_type == 2)
	{
		for (unsigned int current_index = 0; current_index < count; current_index++)
		{
			float * current_data = (float *)(particle_data + current_index * (vertex_size + color_size));
			float * current_storage = storage_data + current_index * 4;

			current_storage[0] = current_data[0];
			current_storage[1] = current_data[1];
			current_storage[2] = current_data[2];
			current_storage[3] = 0;

			if (current_storage[0] < min_x) min_x = current_storage[0];
			if (current_storage[0] > max_x) max_x = current_storage[0];
			if (current_storage[1] < min_y) min_y = current_storage[1];
			if (current_storage[1] > max_y) max_y = current_storage[1];
			if (current_storage[2] < min_z) min_z = current_storage[2];
			if (current_storage[2] > max_z) max_z = current_storage[2];
		}

		float x_range = max_x - min_x;
		float y_range = max_y - min_y;
		float z_range = max_z - min_z;

		std::cout << "x_range:" << x_range << " max_x:" << max_x << " min_x:" << min_x << std::endl;
		std::cout << "y_range:" << y_range << " max_y:" << max_y << " min_y:" << min_y << std::endl;
		std::cout << "z_range:" << z_range << " max_z:" << max_z << " min_z:" << min_z << std::endl;

		float max_range = glm::max(x_range, glm::max(y_range, z_range));

		float x_offset = (max_range - x_range) / 2;
		float y_offset = (max_range - y_range) / 2;
		float z_offset = (max_range - z_range) / 2;

		for (unsigned int current_index = 0; current_index < count; current_index++)
		{
			float * current_storage = storage_data + current_index * 4;

			current_storage[0] = (current_storage[0] - min_x + x_offset) / max_range;
			current_storage[1] = (current_storage[1] - min_y + y_offset) / max_range;
			current_storage[2] = (current_storage[2] - min_y + z_offset) / max_range;
			
			// normalize to 0.8 to center the data in the voxel grid

			for (unsigned int i = 0; i < 3; i++) current_storage[i] = current_storage[i] * 0.8 + 0.1;
		}
		
		radius /= max_range;
	}

	delete[] particle_data;

	data.set(count * 4 * sizeof(float), storage_data);

	delete[] storage_data;

	file.close();
}

unsigned int particle_data::get_count()
{
	return count;
}

float particle_data::get_radius()
{
	return radius;
}

storage & particle_data::get_data()
{
	return data;
}