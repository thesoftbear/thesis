#include "application.hpp"
#include "error.hpp"

#include <iostream>

void MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
}

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

	// OpenGL initialization

	gladLoadGL();

	glGetError();

	const GLubyte * vendor = glGetString(GL_VENDOR);
	std::cout << "Vendor: " << vendor << std::endl;
	
	const GLubyte * renderer = glGetString(GL_RENDERER);
	std::cout << "Renderer: " << renderer << std::endl;

	lastStep = chrono::high_resolution_clock::now();

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback((GLDEBUGPROC)MessageCallback, 0);
}

application::~application()
{
	// glfw termination

	glfwTerminate();
}

bool application::step()
{
	// update camera

	auto now = chrono::high_resolution_clock::now();

	float elapsed = chrono::duration_cast<chrono::microseconds>(now - lastStep).count() / 1000.f;

	lastStep = now;

	// swap buffers

	glfwSwapBuffers(window);
	
	// clear buffer

	glClear(GL_COLOR_BUFFER_BIT);

	// poll window events

	glfwPollEvents();

	// finish main loop once window closes

	return !glfwWindowShouldClose(window);
}