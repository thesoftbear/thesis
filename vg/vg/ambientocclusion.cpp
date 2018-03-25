#include "ambientocclusion.hpp"
#include "error.hpp"

#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL

#include <gtc/matrix_transform.hpp>
#include "gtx\quaternion.hpp"
#include "glm.hpp"
#include "gtc/constants.hpp"

ambientocclusion::ambientocclusion()
{
	draw_geometry_shader.source("../vg/draw_particles.glvs", "../vg/draw_particles.glfs");
	ray_casting.source("../vg/fullscreen_quad.glvs", "../vg/ray_casting.glfs");
	draw_occlusion_shader.source("../vg/fullscreen_quad.glvs", "../vg/draw_occlusion.glfs");

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

	glGenTextures(1, &occlusion_texture);
	glBindTexture(GL_TEXTURE_2D, occlusion_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenFramebuffers(1, &framebuffer);
	glGenRenderbuffers(1, &renderbuffer);
	glGenVertexArrays(1, &vao);

	ray_samples = 0;
}

ambientocclusion::~ambientocclusion()
{
	glDeleteTextures(1, &position_texture);
	glDeleteTextures(1, &normal_texture);

	glDeleteRenderbuffers(1, &renderbuffer);
	glDeleteFramebuffers(1, &framebuffer);

	glDeleteVertexArrays(1, &vao);
}

void ambientocclusion::draw_geometry(float time, particles & p)
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, position_texture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normal_texture, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1280, 720);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);

	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) error("framebuffer incomplete");

	glBindVertexArray(vao);

	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glEnable(GL_DEPTH_TEST);

	mat4 translation = translate(mat4(1.f), vec3(-0.5f, -0.5f, -0.5f));
	mat4 inverseTranslation = translate(mat4(1.f), vec3(0.5f, 0.5f, 0.5f));

	model_rotation += time / 10000.f;

	mat4 rotation = toMat4(quat(vec3(0, model_rotation, 0)));
	mat4 inverseRotation = transpose(rotation);

	mat4 model = rotation * translation;
	mat4 inverseModel = inverseTranslation * inverseRotation;

	mat4 view = lookAt(vec3(0, 0, 1.5f), vec3(0, 0, 0), vec3(0, 1, 0));

	mat4 projection = perspective(radians(65.f), 1280.f / 720.f, 0.01f, 10.f);

	draw_geometry_shader.use();
	p.data().bind(0);
	draw_geometry_shader.set("modelMatrix", model);
	draw_geometry_shader.set("inverseModelMatrix", inverseModel);
	draw_geometry_shader.set("viewMatrix", view);
	draw_geometry_shader.set("projectionMatrix", projection);
	draw_geometry_shader.set("particleSize", p.size());
	draw_geometry_shader.execute(GL_POINTS, 0, p.number());

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	ray_samples = 0;
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

	float pitch = distribution(generator) * glm::pi<float>() / 2.f - glm::pi<float>() / 4.f;
	float roll = distribution(generator) * glm::pi<float>();

	glBindVertexArray(vao);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, position_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normal_texture);

	glBindImageTexture(0, occlusion_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

	ray_casting.use();

	h.get_cell_info().bind(2);
	h.get_particle_data().bind(3);

	std::cout << "sample:" << ray_samples << std::endl;

	ray_casting.set("pitch", pitch);
	ray_casting.set("roll", roll);
	ray_casting.set("cell_count", h.get_resolution());
	ray_casting.set("cell_size", 1.f / h.get_resolution());
	ray_casting.set("particle_size", h.get_particle_size());
	ray_casting.set("ray_samples", ray_samples);
	ray_casting.set("occlusion_texture", 0);

	ray_casting.execute(GL_TRIANGLE_STRIP, 0, 4);

	ray_samples += 1;
}

void ambientocclusion::draw_occlusion()
{
	glBindVertexArray(vao);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, position_texture);

	glBindImageTexture(0, occlusion_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

	draw_occlusion_shader.use();
	ray_casting.set("occlusion_texture", 0);
	ray_casting.set("ray_samples", ray_samples);

	ray_casting.execute(GL_TRIANGLE_STRIP, 0, 4);
}