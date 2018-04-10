﻿#include "application.hpp"
#include "error.hpp"

#include <iostream>

void MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	cerr << "GL " << (type == GL_DEBUG_TYPE_ERROR ? "ERROR: " : "MESSAGE: ") << severity << " " << message << endl;
}

application::application()
{
	// glfw initialization

	if (!glfwInit()) error("glfw initialization failed");

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(1280, 720, "Particle Ambient Occlusion", NULL, NULL);
	
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

	state.elapsed = chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - start_time).count() / 1000.f;

	// update input

	state.left_pressed = glfwGetKey(window, GLFW_KEY_LEFT);
	state.right_pressed = glfwGetKey(window, GLFW_KEY_RIGHT);
	state.up_pressed = glfwGetKey(window, GLFW_KEY_UP);
	state.down_pressed = glfwGetKey(window, GLFW_KEY_DOWN);
	state.in_pressed = glfwGetKey(window, GLFW_KEY_I);
	state.out_pressed = glfwGetKey(window, GLFW_KEY_O);
	state.r_pressed = glfwGetKey(window, GLFW_KEY_R);
	state.v_pressed = glfwGetKey(window, GLFW_KEY_V);

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