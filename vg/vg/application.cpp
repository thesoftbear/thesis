#include "application.hpp"
#include "error.hpp"

application::application()
{
	// glfw initialization

	if (!glfwInit()) error("glfw initialization failed");

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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