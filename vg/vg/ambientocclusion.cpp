#include "ambientocclusion.hpp"

ambientocclusion::ambientocclusion()
{
	ray_casting.source("../vg/fullscreen_quad.glvs", "../vg/ray_casting.glfs");
}

void ambientocclusion::trace_cones(geometry_buffer & g, voxelgrid & v)
{

}

void ambientocclusion::cast_rays(geometry_buffer & g, hashgrid & h)
{
	default_random_engine generator;
	uniform_real_distribution<float> distribution(0.f, 1.f);

	vec3 random_direction = vec3(distribution(generator), distribution(generator), distribution(generator)); // normalize

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g.get_position_texture());
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g.get_normal_texture());

	ray_casting.use();

	ray_casting.execute(GL_TRIANGLE_STRIP, 0, 4);

	glDeleteVertexArrays(1, &vao);
}