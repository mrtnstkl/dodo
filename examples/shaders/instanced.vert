#version 430 core


// =====( INPUT )===== //

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in mat4 instance_transform;


// =====( UNIFORMS )===== //

layout(location = 1) uniform mat4 model_matrix;
layout(location = 2) uniform mat4 view_proj_matrix;


// =====( OUTPUT )===== //

out vertex_data {
    vec3 world_position;
    vec3 world_normal;
    vec2 uv;
} vert;


// =====( MAIN )===== //

void main() {

    vert.world_position = vec3(instance_transform * model_matrix * vec4(position, 1));
    vert.world_normal = normalize(transpose(inverse(mat3(model_matrix))) * normal);
    vert.uv = uv;

    gl_Position = view_proj_matrix * instance_transform * model_matrix * vec4(position, 1);
}
