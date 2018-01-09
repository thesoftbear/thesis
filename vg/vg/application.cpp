#include "application.hpp"
#include "error.hpp"

application::application()
{
	// glfw initialization

	if (!glfwInit()) error("glfw initialization failed");

	window = glfwCreateWindow(1280, 720, "Scattering Test", NULL, NULL);
	
	if (!window) error("glfw window creation failed");

	glfwMakeContextCurrent(window);

	// opengl initialization

	gladLoadGL();
}

application::~application()
{
	// glfw termination

	glfwTerminate();
}

bool application::step()
{
	// swap buffers

	glfwSwapBuffers(window);
	
	// clear buffer

	glClear(GL_COLOR_BUFFER_BIT);

	// poll window events

	glfwPollEvents();

	// finish main loop once window closes

	return !glfwWindowShouldClose(window);
}