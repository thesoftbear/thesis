#include "particles.hpp"
#include "error.hpp"

#include <random>
#include <fstream>
#include <iostream>
#include <chrono>

#define GLM_ENABLE_EXPERIMENTAL

#include "gtc\matrix_transform.hpp"
#include "gtx\quaternion.hpp"
#include "glm.hpp"

particles::particles()
{
	_number = 0;
	_size = 0.f;

	_draw.source("../vg/draw_particles.glvs", "../vg/draw_particles.glfs");
}

particles::~particles()
{
}

void particles::generate(unsigned int frames, unsigned int number, float size)
{
	_frames = frames;
	_current = 0;
	_number = number;
	_size = size;

	float * p = new float[4 * number];

	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution(0.f, 1.f);

	cout << "Generating particles ";

	for (unsigned int particle = 0; particle < number; particle++)
	{
		float * coordinates = p + 4 * particle;

		coordinates[0] = distribution(generator);
		coordinates[1] = distribution(generator);
		coordinates[2] = distribution(generator);

		// if (particle % (number / 10) == 0) cout << ".";
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

	unsigned long long * offsets = new unsigned long long[header.frames];
	file.read((char *)offsets, sizeof(unsigned long long) * header.frames);

	unsigned long long size;
	file.read((char *)&size, sizeof(unsigned long long));

	_offset.clear();
	_offset.reserve(header.frames);

	_coordinates.clear();

	float x_range = header.bounding_box[3] - header.bounding_box[0];
	float y_range = header.bounding_box[4] - header.bounding_box[1];
	float z_range = header.bounding_box[5] - header.bounding_box[2];
	float max_range = (x_range > y_range) ? ((z_range > x_range) ? z_range : x_range) : ((z_range > y_range) ? z_range : y_range);

	for (unsigned int frame = 0; frame < header.frames && frame < 1; frame++)
	{	
		std::cout << "frame " << frame << " / " << header.frames << std::endl;

		if (header.version >= 102)
		{
			float timestamp;
			file.read((char *)&timestamp, sizeof(float));
		}

		unsigned int lists;
		file.read((char *)&lists, sizeof(unsigned int));

		_coordinates.clear();

		for (unsigned int list = 0; list < lists; list++)
		{
			unsigned char vertex_type;
			file.read((char *)&vertex_type, sizeof(unsigned char));

			unsigned char color_type;
			file.read((char *)&color_type, sizeof(unsigned char));

			float radius = 1.f;

			if (vertex_type == 1 || vertex_type == 3)
			{
				file.read((char *)&radius, sizeof(float));
				_size = radius / max_range;
			}

			float minimum_intensity = 0;
			float maximum_intensity = 0;

			if (color_type == 0)
			{
				// read global color
			}
			else if (color_type == 3)
			{
				file.read((char *)&minimum_intensity, sizeof(float));
				file.read((char *)&maximum_intensity, sizeof(float));
			}

			unsigned long long count = 0;
			file.read((char *)&count, sizeof(unsigned long long));
			std::cout << "count = " << count << std::endl;

			float * data = new float[count * 4];
			file.read((char *)data, sizeof(float) * 4 * count);

			(frame > 0) ? _offset.push_back(_offset[frame - 1]) : _offset.push_back(0);
			_coordinates.reserve(_coordinates.size() + count * 4);
			_number = count;

			float min = 1;
			float max = 0;

			if (frame == 0)
			{
				for (unsigned int p = 0; p < count; p++)
				{
					float * x = data + p * 4;
					float * y = x + 1;
					float * z = y + 1;

					_coordinates.push_back((*x - header.bounding_box[0]) / max_range);
					_coordinates.push_back((*y - header.bounding_box[1]) / max_range);
					_coordinates.push_back((*z - header.bounding_box[2]) / max_range);
					_coordinates.push_back(0);
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

		float * start = _coordinates.data() + _offset[frame] * 4 * sizeof(float);

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

void particles::draw(float time, geometry_buffer & g)
{
	GLuint framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g.get_position_texture(), 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, g.get_normal_texture(), 0);

	GLuint renderbuffer;
	glGenRenderbuffers(1, &renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1280, 720);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);

	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) error("framebuffer incomplete");

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glEnable(GL_DEPTH_TEST);

	mat4 translation = translate(mat4(1.f), vec3(-0.5f, -0.5f, -0.5f));

	_rotation += time / 3000.f;
	
	mat4 rotation =	toMat4(quat(vec3(0, _rotation, 0)));

	mat4 model = rotation * translation;

	mat4 view = lookAt(vec3(0, 0, 1.5f), vec3(0, 0, 0), vec3(0, 1, 0));

	mat4 projection = perspective(radians(65.f), 1280.f / 720.f, 0.01f, 10.f);

	_draw.use();
	_data.bind(0);
	_draw.set("modelMatrix", model);
	_draw.set("viewMatrix", view);
	_draw.set("projectionMatrix", projection);
	_draw.set("particleSize", _size);
	_draw.execute(GL_POINTS, 0, _number);

	glDeleteVertexArrays(1, &vao);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &framebuffer);
}