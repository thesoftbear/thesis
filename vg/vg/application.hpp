#pragma once

#include "glad\glad.h"
#include "GLFW\glfw3.h"

#include <chrono>

using namespace std;

class application
{
	public:

		application();
		~application();

		bool step();
		float elapsed();
		void swap();

	private:

		GLFWwindow * window;
		chrono::high_resolution_clock::time_point start_time;
		float _elapsed;
};