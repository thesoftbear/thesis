#include "ambientocclusion.hpp"
#include "error.hpp"

#define GLM_ENABLE_EXPERIMENTAL

#include <gtc/matrix_transform.hpp>
#include "gtx\quaternion.hpp"
#include "glm.hpp"

ambientocclusion::ambientocclusion()
{
	draw_geometry_shader.source("../vg/draw_particles.glvs", "../vg/draw_particles.glfs");
	ray_casting.source("../vg/fullscreen_quad.glvs", "../vg/ray_casting.glfs");

	unsigned int width = 1280;
	unsigned int height = 720;

	glGenTextures(1, &position_texture);
	glBindTexture(GL_TEXTURE_2D, position_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenTextures(1, &normal_texture);
	glBindTexture(GL_TEXTURE_2D, normal_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

ambientocclusion::~ambientocclusion()
{
	glDeleteTextures(1, &position_texture);
	glDeleteTextures(1, &normal_texture);
}

void ambientocclusion::draw_geometry(float time, particles & p)
{
	GLuint framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, position_texture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normal_texture, 0);

	GLuint renderbuffer;
	glGenRenderbuffers(1, &renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1280, 720);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);

	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) error("framebuffer incomplete");

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glEnable(GL_DEPTH_TEST);

	mat4 translation = translate(mat4(1.f), vec3(-0.5f, -0.5f, -0.5f));

	model_rotation += time / 3000.f;

	mat4 rotation = toMat4(quat(vec3(0, model_rotation, 0)));

	mat4 model = rotation * translation;

	mat4 view = lookAt(vec3(0, 0, 1.5f), vec3(0, 0, 0), vec3(0, 1, 0));

	mat4 projection = perspective(radians(65.f), 1280.f / 720.f, 0.01f, 10.f);

	draw_geometry_shader.use();
	p.data().bind(0);
	draw_geometry_shader.set("modelMatrix", model);
	draw_geometry_shader.set("viewMatrix", view);
	draw_geometry_shader.set("projectionMatrix", projection);
	draw_geometry_shader.set("particleSize", p.size());
	draw_geometry_shader.execute(GL_POINTS, 0, p.number());

	glDeleteVertexArrays(1, &vao);
	glDeleteRenderbuffers(1, &renderbuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &framebuffer);
}

void ambientocclusion::draw_geometry(float time, hashgrid & h)
{

}

void ambientocclusion::trace_cones(voxelgrid & v)
{

}

void ambientocclusion::cast_rays(hashgrid & h)
{
	uniform_real_distribution<float> distribution(0.f, 1.f);

	float x_angle = distribution(generator) * 180.f - 90.f;
	float y_angle = distribution(generator) * 180.f - 90.f;

	mat4 rotation(1.f);
	rotation = rotate(rotation, x_angle, vec3(1, 0, 0));
	rotation = rotate(rotation, y_angle, vec3(0, 1, 0));

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, position_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normal_texture);

	ray_casting.use();

	ray_casting.set("rotation", rotation);

	ray_casting.execute(GL_TRIANGLE_STRIP, 0, 4);

	glDeleteVertexArrays(1, &vao);
}