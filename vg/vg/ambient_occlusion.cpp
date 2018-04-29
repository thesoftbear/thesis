#include "ambient_occlusion.hpp"
#include "error.hpp"

#define GLM_ENABLE_EXPERIMENTAL

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/constants.hpp>
#include <gtc/quaternion.hpp>
#include <gtx/quaternion.hpp>
#include <gtx/transform.hpp>

#include <iostream>

ambient_occlusion::ambient_occlusion()
{
	draw_geometry_shader.source("../vg/draw_geometry.glvs", "../vg/draw_geometry.glfs");
	cast_rays_shader.source("../vg/fullscreen_quad.glvs", "../vg/cast_rays.glfs");
	trace_cones_shader.source("../vg/fullscreen_quad.glvs", "../vg/trace_cones.glfs");
	draw_occlusion_shader.source("../vg/fullscreen_quad.glvs", "../vg/draw_occlusion.glfs");

	glGenTextures(1, &position_texture);
	glGenTextures(1, &normal_texture);
	glGenTextures(1, &occlusion_texture);

	glGenFramebuffers(1, &framebuffer);
	glGenRenderbuffers(1, &renderbuffer);
	glGenVertexArrays(1, &vao);

	samples = 0;
	ray_casting = false;
	distance = 0.03f; // 625
	orientation = angleAxis(0.f, vec3(0));
}

ambient_occlusion::~ambient_occlusion()
{
	glDeleteTextures(1, &position_texture);
	glDeleteTextures(1, &normal_texture);
	glDeleteTextures(1, &occlusion_texture);
	glDeleteRenderbuffers(1, &renderbuffer);
	glDeleteFramebuffers(1, &framebuffer);
	glDeleteVertexArrays(1, &vao);
}

bool ambient_occlusion::update_view_configuration(application_state & s)
{
	// update geometry buffer & resolution

	if (s.resolution_changed)
	{
		resolution.x = s.framebuffer_width;
		resolution.y = s.framebuffer_height;

		glBindTexture(GL_TEXTURE_2D, position_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, resolution.x, resolution.y, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glBindTexture(GL_TEXTURE_2D, normal_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, resolution.x, resolution.y, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glBindTexture(GL_TEXTURE_2D, occlusion_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, resolution.x, resolution.y, 0, GL_RED, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glViewport(0, 0, resolution.x, resolution.y);

		glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, resolution.x, resolution.y);
	}

	// update object rotation & distance

	float last_distance = distance;
	quat last_orientation = orientation;

	float velocity = 0.01f;
	float scale = 0.1 + pow((glm::min(float(distance), 0.9f) / 0.9f), 2.0); // scale velocity based on distance

	if (s.in_pressed) distance -= velocity * scale;
	if (s.out_pressed) distance += velocity * scale;
	distance = glm::min(glm::max(distance, 0.01f), 2.5f);

	vec2 rotation_angles(0);
	if (s.left_pressed) rotation_angles.y = -velocity;
	if (s.right_pressed) rotation_angles.y = velocity;
	if (s.up_pressed) rotation_angles.x = -velocity;
	if (s.down_pressed) rotation_angles.x = velocity;

	quat x_rotation = angleAxis(rotation_angles.x, vec3(1, 0, 0));
	quat y_rotation = angleAxis(rotation_angles.y, vec3(0, 1, 0));

	orientation = x_rotation * y_rotation * orientation;

	if (last_distance != distance || last_orientation != orientation || s.resolution_changed) return true;
	else return false;
}

void ambient_occlusion::update_geometry_buffer(application_state s, particle_data & p)
{
	bool view_changed = update_view_configuration(s);

	if (!view_changed) return;

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, position_texture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normal_texture, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);

	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);
	
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(vao);

	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glEnable(GL_DEPTH_TEST);

	draw_geometry_shader.use();
	
	p.get_data().bind(0);

	mat4 translation = translate(mat4(1.f), vec3(-0.5f, -0.5f, -0.5f));
	mat4 rotation = toMat4(orientation);
	mat4 model_matrix = rotation * translation;
	draw_geometry_shader.set("modelMatrix", model_matrix);

	mat4 inverse_translation = translate(mat4(1.f), vec3(0.5f, 0.5f, 0.5f));
	mat4 inverse_rotation = transpose(rotation);
	mat4 inverse_model_matrix = inverse_translation * inverse_rotation;
	draw_geometry_shader.set("inverseModelMatrix", inverse_model_matrix);

	mat4 view_matrix = lookAt(vec3(0, 0, distance), vec3(0, 0, 0), vec3(0, 1, 0));
	draw_geometry_shader.set("viewMatrix", view_matrix);

	mat4 projection_matrix = perspective(radians(65.f), float(s.framebuffer_width) / float(s.framebuffer_height), 0.01f, 10.f);
	draw_geometry_shader.set("projectionMatrix", projection_matrix);

	draw_geometry_shader.set("particleSize", p.get_radius());
	draw_geometry_shader.set("resolution", resolution);

	draw_geometry_shader.execute(GL_POINTS, 0, p.get_count());

	samples = 0;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ambient_occlusion::trace_cones(voxel_octree & v, unsigned int subdivision_factor)
{
	glBindVertexArray(vao);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, position_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normal_texture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_3D, v.get_texture());

	trace_cones_shader.use();

	glBindImageTexture(0, occlusion_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

	float opening_angle = 40.f;

	trace_cones_shader.set("occlusion_texture", 0);
	trace_cones_shader.set("opening_angle", radians(opening_angle));
	trace_cones_shader.set("voxel_size", 1.f / v.get_resolution());
	trace_cones_shader.set("resolution", resolution);

	trace_cones_shader.execute(GL_TRIANGLE_STRIP, 0, 4);

	samples = 1.f;
	ray_casting = false;
}

std::vector<vec4> subdivide_hemisphere(unsigned int factor)
{
	float angle = 180.f / factor;


}

void ambient_occlusion::cast_rays(space_partitioning_grid & h)
{
	uniform_real_distribution<float> distribution(0.f, 1.f);

	float pitch = acos(distribution(random_generator));
	float roll = distribution(random_generator) * glm::pi<float>() * 2.f;

	glBindVertexArray(vao);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, position_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normal_texture);

	glBindImageTexture(0, occlusion_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

	cast_rays_shader.use();

	h.get_cell_info().bind(2);
	h.get_particle_storage().bind(3);

	cast_rays_shader.set("pitch", pitch);
	cast_rays_shader.set("roll", roll);
	cast_rays_shader.set("cell_count", h.get_resolution());
	cast_rays_shader.set("cell_size", 1.f / h.get_resolution());
	cast_rays_shader.set("particle_size", h.get_particle_radius());
	cast_rays_shader.set("ray_samples", samples);
	cast_rays_shader.set("resolution", resolution);

	cast_rays_shader.execute(GL_TRIANGLE_STRIP, 0, 4);

	if (!ray_casting)
	{
		ray_casting = true;
		samples = 0;
	}

	samples += glm::cos(pitch);
}

void ambient_occlusion::draw_occlusion()
{
	glBindVertexArray(vao);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, position_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normal_texture);

	glBindImageTexture(0, occlusion_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

	draw_occlusion_shader.use();
	draw_occlusion_shader.set("occlusion_texture", 0);
	draw_occlusion_shader.set("samples", samples);
	draw_occlusion_shader.set("resolution", resolution);
	draw_occlusion_shader.execute(GL_TRIANGLE_STRIP, 0, 4);
}