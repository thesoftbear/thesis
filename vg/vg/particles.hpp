#pragma once

#include "storage.hpp"

#include <string>

using namespace std;

class particles
{
	public:
		
		particles();
		~particles();
		void generate(unsigned int number, float size);
		void read(string path);
		unsigned int number();
		float size();
		storage & data();

	private:

		unsigned int _number;
		float _size;
		storage _data;
};