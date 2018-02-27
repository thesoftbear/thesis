#pragma once

#include "glad\glad.h"
#include "glm.hpp"

#include <string>

using namespace std;
using namespace glm;
#define glsl(...) #__VA_ARGS__

class shader
{
	public:

		shader();
		~shader();
		void source(string a); // compute shader
		void source(string a, string b); // vertex shader + fragment shader
		void source(string a, string b, string c); // vertex shader + geometry shader + fragment shader
		void use();
		void set(string name, float value);
		void set(string name, unsigned int value);
		void set(string name, mat4 value);
		void execute(unsigned int x, unsigned int y = 1, unsigned int z = 1);

	private:
	
		GLuint compile(string source);
		void link();

		GLuint id;
		bool compute;
};