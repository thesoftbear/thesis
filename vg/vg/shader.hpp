#pragma once

#include "glad\glad.h"

#include <string>

using namespace std;

class shader
{
	public:

		shader(string a); // compute shader
		shader(string a, string b); // vertex shader + fragment shader
		~shader();

		void set(string name, float value);
		void use();
		void execute(unsigned int x, unsigned int y, unsigned int z);

	private:
	
		GLuint compile(string source);
		void link();

		GLuint id;
};