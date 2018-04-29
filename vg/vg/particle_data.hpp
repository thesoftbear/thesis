#pragma once

#include "storage.hpp"
#include "shader.hpp"

#include <string>
#include <vector>

using namespace std;

class particle_data
{
	public:

		particle_data();
		~particle_data();
		void generate_random(unsigned int count, float radius);
		void generate_two(float radius, float distance);
		void read_file(string path, unsigned int frame);
		unsigned int get_count();
		float get_radius();
		storage & get_data();

	private:

		unsigned int count;
		float radius;
		storage data;
};