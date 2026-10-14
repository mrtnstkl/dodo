#include "DefaultShaders.h"
#include "Shader.h"

using namespace dodo::shader;

shader_program* dodo::default_shaders::basic()
{
	const auto vs = std::make_shared<shader::shader>(vertex_shader,
R"(#version 430 core

in vec3 position;
in vec3 normal;

uniform mat4 model_matrix;
uniform mat4 view_proj_matrix;

void main() {
	gl_Position = view_proj_matrix * model_matrix * vec4(position, 1);
})", source_code, false);
	const auto fs = std::make_shared<shader::shader>(fragment_shader,
R"(#version 430 core
out vec4 fragColor;
void main() {
	fragColor = vec4(0.5,0.2,0.8,1);
}
)", source_code, false);
	return new shader_program(vs, fs, true, true);
}

shader_program* dodo::default_shaders::textured_mesh()
{
	const auto vs = std::make_shared<shader::shader>(vertex_shader, R"(#version 430 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

layout(location = 1) uniform mat4 model_matrix;
layout(location = 2) uniform mat4 view_proj_matrix;

out vertex_data {
	vec3 world_normal;
	vec2 uv_coordinate;
} vert;

void main() {
	vert.world_normal = normalize(transpose(inverse(mat3(model_matrix))) * normal);
	vert.uv_coordinate = uv;
	gl_Position = view_proj_matrix * model_matrix * vec4(position, 1);
}
)", source_code, false);
	const auto fs = std::make_shared<shader::shader>(fragment_shader, R"(#version 430 core
in vertex_data {
	vec3 world_normal;
	vec2 uv_coordinate;
} vert;

layout(location = 30) uniform sampler2D color_texture;

layout(location = 0) out vec4 fragColor;

void main() {
	fragColor = texture(color_texture, vert.uv_coordinate);
	fragColor.a = 0.5;
}
)", source_code, false);
	return new shader_program(vs, fs, true, true);
}

shader_program* dodo::default_shaders::skybox()
{
	const auto vs = std::make_shared<shader::shader>(vertex_shader, R"(#version 430 core
in vec3 position;
uniform mat4 view_proj_matrix;
out vertex_data {
	vec3 cube_coordinate;
} vert;

void main() {
	vert.cube_coordinate = (vec4(position.x, position.y, position.z, 1)).xyz;
	gl_Position = (view_proj_matrix * vec4(position, 1.0)).xyww;
}
)", source_code, false);
	const auto fs = std::make_shared<shader::shader>(fragment_shader, R"(#version 430 core
in vertex_data {
	vec3 cube_coordinate;
} vert;
uniform samplerCube environment_map;
out vec4 fragColor;

void main() {
	fragColor = texture(environment_map, vert.cube_coordinate);
}
)", source_code, false);
	return new shader_program(vs, fs, true, true);
}
