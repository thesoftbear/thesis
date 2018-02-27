#pragma once

#include "glm.hpp"

using namespace glm;

class Camera
{
	public:

		Camera();
		void setPosition(vec3 position);
		vec3 getPosition();
		void movePosition(vec3 movement);
		void setDirection(vec3 direction);
		vec3 getDirection();
		void rotateDirection(float horizontal, float vertical);
		mat4 getMatrix();

	private:

		vec3 position;
		vec3 direction;
};