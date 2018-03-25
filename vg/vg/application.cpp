#include "application.hpp"
#include "error.hpp"

#include <iostream>

void MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	// cerr << "GL " << (type == GL_DEBUG_TYPE_ERROR ? "ERROR: " : "MESSAGE: ") << severity << " " << message << endl;
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

	start_time = chrono::high_resolution_clock::now();

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
	// update time

	auto now = chrono::high_resolution_clock::now();

	_elapsed = chrono::duration_cast<chrono::microseconds>(now - start_time).count() / 1000.f;

	swap();

	// poll window events

	glfwPollEvents();

	// finish main loop once window closes

	return !glfwWindowShouldClose(window);
}

void application::swap()
{
	// swap buffers

	glfwSwapBuffers(window);

	// clear buffer

	glClearColor(0, 0, 0, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

float application::elapsed()
{
	return _elapsed;
}