#include "particles.hpp"

#include <random>
#include <fstream>
#include <iostream>

particles::particles()
{
	_number = 0;
	_size = 0.f;
}

particles::~particles()
{

}

void particles::generate(unsigned int number, float size)
{
	_number = number;
	_size = size;

	float * p = new float[3 * number];

	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution(0.f, 1.f);

	for (unsigned int particle = 0; particle < number; particle++)
	{
		float * coordinates = p + 3 * particle;

		coordinates[0] = distribution(generator);
		coordinates[1] = distribution(generator);
		coordinates[2] = distribution(generator);
	}

	_data.set(3 * number * sizeof(float), p);

	delete[] p;
}

void particles::read(string path)
{
	// read from mmpld file ...

	ifstream file(path);

	struct
	{
		char identifier[6];
		unsigned short version;
		unsigned int frames;
		float bounding_box[6];
		float clipping_box[6];
	}
	header;

	file.read((char *)&header, sizeof(header));

	unsigned long long * offsets = new unsigned long long[header.frames];

	file.read((char *)offsets, sizeof(unsigned long long) * header.frames);

	unsigned long long size;

	file.read((char *)&size, sizeof(unsigned long long));


}

unsigned int particles::number()
{
	return _number;
}

float particles::size()
{
	return _size;
}

storage & particles::data()
{
	return _data;
}