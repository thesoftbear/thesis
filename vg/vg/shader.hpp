#pragma once

#include "glad\glad.h"

#include <string>

using namespace std;

class shader
{
	public:

		shader(string a);
		shader(string a, string b);
		~shader();

		void set(string name, float value);
		void use();

	private:
	
		GLuint compile(string source);
		void link();

		GLuint id;
};