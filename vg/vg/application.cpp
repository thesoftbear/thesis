#include "application.hpp"
#include "error.hpp"

#include <iostream>

void message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	if(type == GL_DEBUG_TYPE_ERROR) cerr << "GL ERROR: " << severity << " " << message << endl;
}

application::application()
{
	// glfw initialization

	if (!glfwInit()) error("glfw initialization failed");

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(1000, 1000, "Particle Ambient Occlusion", NULL, NULL);
	
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
	glDebugMessageCallback((GLDEBUGPROC)message_callback, 0);

	glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);
}

application::~application()
{
	// glfw termination

	glfwTerminate();
}

bool application::step()
{
	// update time

	state.milliseconds_elapsed = chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - start_time).count() / 1000.f;

	// update input

	state.left_pressed = glfwGetKey(window, GLFW_KEY_LEFT);
	state.right_pressed = glfwGetKey(window, GLFW_KEY_RIGHT);
	state.up_pressed = glfwGetKey(window, GLFW_KEY_UP);
	state.down_pressed = glfwGetKey(window, GLFW_KEY_DOWN);
	state.in_pressed = glfwGetKey(window, GLFW_KEY_I);
	state.out_pressed = glfwGetKey(window, GLFW_KEY_O);
	state.r_pressed = glfwGetKey(window, GLFW_KEY_R);
	state.v_pressed = glfwGetKey(window, GLFW_KEY_V);

	// update window size

	state.resolution_changed = false;

	int new_width, new_height;
	glfwGetFramebufferSize(window, &new_width, &new_height);

	if (new_width != state.framebuffer_width)
	{
		state.framebuffer_width = new_width;
		state.resolution_changed = true;
	} 
	
	if (new_height != state.framebuffer_height)
	{
		state.framebuffer_height = new_height;
		state.resolution_changed = true;
	}

	if(state.resolution_changed) glViewport(0, 0, new_width, new_height);

	// swap buffers

	glfwSwapBuffers(window);

	// clear buffers

	glClearColor(0, 0, 0, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// poll window events

	glfwPollEvents();

	// finish main loop once window closes

	return !glfwWindowShouldClose(window);
}

application_state application::get_state()
{
	return state;
}