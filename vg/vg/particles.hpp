#pragma once

#include "storage.hpp"
#include "shader.hpp"
#include "geometry_buffer.hpp"

#include <string>
#include <vector>

using namespace std;

class particles
{
	public:

		particles();
		~particles();
		void generate(unsigned int frames, unsigned int number, float size);
		void read(string path);
		void select(unsigned int frame);
		unsigned int frames();
		unsigned int current();
		unsigned int number();
		float size();
		storage & data();
		void draw(float time, geometry_buffer & b);

	private:

		unsigned int _frames;
		unsigned int _current;
		unsigned int _number;
		float _size;
		storage _data;

		vector<unsigned int> _offset;
		vector<float> _coordinates;

		shader _draw;

		float _rotation;
};