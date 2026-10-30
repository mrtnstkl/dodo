/**
 * @file instancing.cpp
 *
 * An example showcasing how to use instanced rendering in Dodo.
 * A custom shader is created for rendering the instanced model, which takes in instance data for position and color.
 * 1000 spheres are rendered with different positions and colors using a single draw call.
 * Dodo provides some instance array structs in the dodo::instanced::array_structs namespace, but here we demonstrate how to create a custom one.
 *
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>

#include <Application.h>
#include <Renderer.h>
#include <Shader.h>
#include <Model.h>
#include <Camera.h>
#include <Mesh.h>
#include <DefaultShaders.h>
#include <Instanced.h>
#include <random>

#define ASSET_PATH "examples/"

class application;

const auto vertex_shader_code = R"(#version 430 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

layout(location = 3) in vec3 instance_pos;
layout(location = 4) in vec3 instance_color;

uniform mat4 model_matrix;
uniform mat4 view_proj_matrix;

out vertex_data {
	vec2 uv;
	vec3 color;
} vert;

void main() {
	vert.uv = uv;
	vert.color = instance_color;
	gl_Position = view_proj_matrix * model_matrix * vec4(position + instance_pos, 1);
}
)";

const auto fragment_shader_code = R"(#version 430 core
in vertex_data {
	vec2 uv;
	vec3 color;
} vert;

uniform sampler2D color_texture;

out vec4 fragColor;

void main() {
	fragColor = texture(color_texture, vert.uv) * vec4(vert.color, 1.0);
}
)";

class application : public dodo::application
{
	dodo::renderer* renderer = nullptr;
	std::shared_ptr<dodo::perspective_camera> camera;

	dodo::shader::shader_program* instanced_shader;

	// we define a custom instance struct for our instanced model, which contains a position and a color for each instance
	// the struct must have a default constructor and a static bind function which configures the vertex attributes for the instance data
	struct custom_instanced_struct : dodo::instanced::array_structs::instanced_array_struct
	{
		glm::vec3 pos;
		glm::vec3 color;

		custom_instanced_struct(glm::vec3 pos = {}, glm::vec3 color = {})
			: pos(pos), color(color) {}

		static void bind(GLuint starting_index)
		{
			// the bind function configures the vertex attributes for the instance data
			glEnableVertexAttribArray(starting_index + 0); // enable position attribute
			glEnableVertexAttribArray(starting_index + 1); // enable color attribute

			const auto stride = sizeof(custom_instanced_struct);
			// position attribute is a vec3 at offset 0, color attribute is a vec3 at offset sizeof(vec3)
			glVertexAttribPointer(starting_index + 0, 3, GL_FLOAT, GL_FALSE, stride, (void*)(0));
			glVertexAttribPointer(starting_index + 1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(glm::vec3)));

			glVertexAttribDivisor(starting_index + 0, 1);
			glVertexAttribDivisor(starting_index + 1, 1);
		}
	};

	dodo::model* spheres;

	struct {
		double x = 0, y = 0;
		bool pressed = false;
	} mouse;

	void scene_init() override;
	void scene_destroy() override;

	void handle_movement_input();

	void update() override;

	void on_cursor_pos(GLFWwindow* window, double x_pos, double y_pos) override;
};


void application::scene_init()
{
	renderer = new dodo::renderer;
	renderer->set_clear_color({0, 0, .2, 1});
	camera = std::make_shared<dodo::perspective_camera>();
	renderer->set_camera(camera);

	// build a custom shader for rendering the instanced model
	instanced_shader = new dodo::shader::shader_program(
		std::make_shared<dodo::shader::shader>(dodo::shader::vertex_shader, vertex_shader_code, dodo::shader::source_code),
		std::make_shared<dodo::shader::shader>(dodo::shader::fragment_shader, fragment_shader_code, dodo::shader::source_code)
	);
	if (!instanced_shader->ready())
	{
		std::cerr << "Failed to compile shader: " << instanced_shader->error_msg() << std::endl;
		error_ = true; return;
	}
	else
	{
		std::cout << "Successfully compiled shader." << std::endl;
		instanced_shader->print_debug_info(std::cout);
	}

	auto material = dodo::materials::standard();
	material.texture = std::make_shared<dodo::texture>(dodo::texture::from_image_file(ASSET_PATH "textures/tiles_diffuse.png"));

	// create an instanced mesh with our custom instance struct
	// we use a sphere mesh and initialize the instance array with 1000 elements
	auto* instanced_mesh =
		new dodo::instanced::instanced_mesh<dodo::vertex_structs::vertex3_norm_uv, custom_instanced_struct>(
			dodo::standard_mesh::generate_sphere(1, 12, 6),
			1000);
	// each instance has a position and a color
	// here we set them to random values
	std::default_random_engine rng;
	std::uniform_real_distribution<float> pos(-50.f, 50.f), col(0.f, 1.f);
	for (auto& instance : *instanced_mesh)
	{
		instance.pos = { pos(rng), pos(rng), pos(rng) };
		instance.color = { col(rng), col(rng), col(rng) };
	}
	// after modifying the instance data, we need to call update_instance_buffer to copy the changes to the GPU
	instanced_mesh->update_instance_buffer();
	spheres = new dodo::model(instanced_mesh, {0, 0, 0}, material);
}

void application::scene_destroy()
{
	delete renderer;
	delete instanced_shader;
	delete spheres;
}

void application::handle_movement_input()
{
	float delta = 0.16f;
	if (glfwGetKey(window_, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		delta *= 10;
	else if (glfwGetKey(window_, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		delta *= 0.5f;
	glm::vec3 forward_vec = camera->view_vector();
	forward_vec.y = 0;
	forward_vec = normalize(forward_vec) * delta;
	glm::vec3 lateral_vec = glm::normalize(glm::cross(forward_vec, {0, 1, 0})) * delta;
	if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS)
		camera->translate(forward_vec);
	if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS)
		camera->translate(-forward_vec);
	if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS)
		camera->translate(-lateral_vec);
	if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS)
		camera->translate(lateral_vec);
	if (glfwGetKey(window_, GLFW_KEY_R) == GLFW_PRESS)
		camera->translate({0, delta, 0});
	if (glfwGetKey(window_, GLFW_KEY_F) == GLFW_PRESS)
		camera->translate({0, -delta, 0});
}

void application::update()
{
	handle_movement_input();

	renderer->clear();
	// the instanced model is rendered just like a regular model
	renderer->render_model(*spheres, *instanced_shader);
}

void application::on_cursor_pos(GLFWwindow* window, double x_pos, double y_pos)
{
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS)
	{
		mouse.pressed = false;
		return;
	}
	if (mouse.pressed)
	{
		const float rotate_angle = -0.005f;
		camera->yaw(rotate_angle * static_cast<float>(mouse.x - x_pos));
		camera->pitch(rotate_angle * -static_cast<float>(mouse.y - y_pos));
		mouse.x = x_pos;
		mouse.y = y_pos;
	}
	else
	{
		mouse.x = x_pos;
		mouse.y = y_pos;
		mouse.pressed = true;
	}
}

int main()
{
	application app;
	app.run();
	return 0;
}
