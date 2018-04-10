#pragma once

#include "glad\glad.h"
#include "GLFW\glfw3.h"

#include <chrono>

using namespace std;

struct application_state
{
	float elapsed;
	bool left_pressed;
	bool right_pressed;
	bool up_pressed;
	bool down_pressed;
	bool in_pressed;
	bool out_pressed;
	bool r_pressed;
	bool v_pressed;
};

class application
{
	public:

		application();
		~application();

		bool step();
		application_state get_state();

	private:

		GLFWwindow * window;
		chrono::high_resolution_clock::time_point start_time;
		application_state state;
};