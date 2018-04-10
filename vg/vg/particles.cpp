#include "particles.hpp"
#include "error.hpp"

#include <random>
#include <fstream>
#include <iostream>
#include <chrono>

#define GLM_ENABLE_EXPERIMENTAL

#include "gtc\noise.hpp"

particles::particles()
{
	_number = 0;
	_size = 0.f;
}

particles::~particles()
{
}

void particles::generate(unsigned int frames, unsigned int number, float size)
{
	_frames = frames;
	_current = 0;
	_size = size;

	float * p = new float[4 * number];

	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution(0.f, 1.f);

	cout << "Generating particles ";

	_number = 0;

	while (_number < number)
	{
		float x = distribution(generator);
		float y = distribution(generator);
		float z = distribution(generator);

		// sphere teste
		float xs = x - 0.5f;
		float ys = y - 0.5f;
		float zs = z - 0.5f;
		if (sqrt(xs * xs + ys * ys + zs * zs) > 0.4f) continue;

		float chance = glm::simplex(glm::vec3(x * 10, y * 10, z * 10));

		float draw = distribution(generator);

		if (draw <= chance)
		{
			float * coordinates = p + 4 * _number;

			coordinates[0] = x;
			coordinates[1] = y;
			coordinates[2] = z;

			_number++;
		}
	}

	_data.set(4 * number * sizeof(float), p, GL_STATIC_DRAW);

	delete[] p;

	cout << " complete." << endl;
}

void particles::read(string path)
{
	ifstream file(path, ios::beg | ios::binary);

	#pragma pack(push, 1)
	struct file_header
	{
		char identifier[6];
		unsigned short version;
		unsigned int frames;
		float bounding_box[6];
		float clipping_box[6];
	};
	#pragma pack(pop)

	file_header header;
	file.read((char *)&header, sizeof(file_header));

	cout << "version: " << header.version << endl;

	unsigned long long * offsets = new unsigned long long[header.frames];
	file.read((char *)offsets, sizeof(unsigned long long) * header.frames);

	unsigned long long size;
	file.read((char *)&size, sizeof(unsigned long long));

	cout << "size: " << size << endl;

	_frames = header.frames;

	cout << "frames: " << header.frames << endl;

	cout << "bounding_box: " << header.bounding_box[0] << " " << header.bounding_box[1] << " " << header.bounding_box[2] << " " << header.bounding_box[3] << " " << header.bounding_box[4] << " " << header.bounding_box[5] << endl;

	_offset.clear();
	_offset.reserve(header.frames);

	_coordinates.clear();
	float y_range = header.bounding_box[4] - header.bounding_box[1];
	float z_range = header.bounding_box[5] - header.bounding_box[2];

	float x_range = header.bounding_box[3] - header.bounding_box[0];

	float max_range = (x_range > y_range) ? ((z_range > x_range) ? z_range : x_range) : ((z_range > y_range) ? z_range : y_range);

	std::cout << "max_range: " << max_range << endl;

	const unsigned int target_frame = 40;

	for (unsigned int frame = target_frame; frame < header.frames && frame <= target_frame; frame++)
	{
		std::cout << "frame " << frame << " / " << header.frames << " (" << file.tellg() << ")" << std::endl;

		file.seekg(offsets[frame]);

		if (header.version >= 102)
		{
			float timestamp;
			file.read((char *)&timestamp, sizeof(float));

			cout << "t:" << timestamp;
		}

		unsigned int lists;
		file.read((char *)&lists, sizeof(unsigned int));

		cout << "l: " << lists << endl;

		for (unsigned int list = 0; list < lists; list++)
		{
			unsigned char vertex_type;
			unsigned char color_type;

			file.read((char *)&vertex_type, sizeof(unsigned char));
			file.read((char *)&color_type, sizeof(unsigned char));

			cout << "v: " << int(vertex_type) << " c: " << int(color_type) << endl;

			float radius = 1.f;

			if (vertex_type == 1 || vertex_type == 3)
			{
				file.read((char *)&radius, sizeof(float));
				_size = radius / max_range;
			}
			else if (vertex_type == 2)
			{
				_size = 0.001;
			}

			cout << "r: " << radius << " (" << _size << ")" << endl;

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

			unsigned long long count = 0;
			file.read((char *)&count, sizeof(unsigned long long));
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

			unsigned char * data = new unsigned char[count * (vertex_size + color_size)];
			file.read((char *)data, count * (vertex_size + color_size));

			const unsigned int factor = 1;

			if (frame == target_frame)
			{
				_coordinates.reserve(_coordinates.size() + count * 4);
				_number = count;

				for (unsigned int p = 0; p < count; p++)
				{
					if (vertex_type == 1 || vertex_type == 2)
					{
						float * d = (float *)(data + p * (vertex_size + color_size));

						float x = ((*d) - header.bounding_box[0]) / max_range;
						float y = ((*(d + 1)) - header.bounding_box[1]) / max_range;
						float z = ((*(d + 2)) - header.bounding_box[2]) / max_range;

						_coordinates.push_back(x);
						_coordinates.push_back(y);
						_coordinates.push_back(z);
						_coordinates.push_back(0);
					}
				}
			}

			delete[] data;
		}
	}

	file.close();
}

unsigned int particles::frames()
{
	return _frames;
}

unsigned int particles::current()
{
	return _current;
}

unsigned int particles::number()
{
	return _number;
}

float particles::size()
{
	return _size;
}

void particles::select(unsigned int frame)
{
	if (0 <= frame && frame <= _frames - 1)
	{
		_current = frame;

		float * start = _coordinates.data();

		_data.set(_number * 4 * sizeof(float), start);
	}
	else
	{
		error("frame does not exist");
	}
}

storage & particles::data()
{
	return _data;
}