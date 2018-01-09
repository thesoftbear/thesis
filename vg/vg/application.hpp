#pragma once

#include "glad\glad.h"
#include "GLFW\glfw3.h"

class application
{
	public:

		application();
		~application();

		bool step();

	private:

		GLFWwindow * window;
};