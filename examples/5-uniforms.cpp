/**
 * @file uniforms.cpp
 *
 * An example showcasing how to use custom shader uniforms.
 * A custom shader is created from source code defined in this file.
 * The fragment shader defines a custom uniform "custom_color" that is not recognized by the renderer.
 * Upon shader compilation, Dodo retrieves the location of this uniform and stores it.
 * In the update loop, we manually set the value of this uniform to a color.
 * Pressing the spacebar randomizes the color.
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
#include <random>

#define ASSET_PATH "examples/"

class application;

const auto vertex_shader_code = R"(#version 430 core
in vec3 position;
in vec3 normal;
in vec2 uv;

uniform mat4 model_matrix;
uniform mat4 view_proj_matrix;

out vertex_data {
	vec3 world_normal;
	vec2 uv_coordinate;
} vert;

void main() {
	vert.world_normal = normalize(transpose(inverse(mat3(model_matrix))) * normal);
	vert.uv_coordinate = uv;
	gl_Position = view_proj_matrix * model_matrix * vec4(position, 1);
}
)";

const auto fragment_shader_code = R"(#version 430 core
in vertex_data {
	vec3 world_normal;
	vec2 uv_coordinate;
} vert;

uniform sampler2D color_texture;

// here we define a custom uniform that is not recognized and therefore ignored by the renderer
// the engine will acknowledge its existence and allow us to manually set it from the application
uniform vec3 custom_color;

out vec4 fragColor;

void main() {
	fragColor = texture(color_texture, vert.uv_coordinate) * vec4(custom_color, 1);
}
)";

class application : public dodo::application
{
	dodo::renderer* renderer = nullptr;
	dodo::shader::shader_program* custom_shader;
	std::shared_ptr<dodo::perspective_camera> camera;
	std::shared_ptr<dodo::arcball_camera_controller> camera_controller;
	dodo::model* sphere;

	struct {
		double x = 0, y = 0;
		bool pressed = false;
	} mouse;

	glm::vec3 custom_color = {1, 0, 1};

	void scene_init() override;
	void scene_destroy() override;

	void update() override;

	void on_key(GLFWwindow* window, int key, int scan_code, int action, int mods) override;
	void on_cursor_pos(GLFWwindow* window, double x_pos, double y_pos) override;
	void on_scroll(GLFWwindow* window, double x_offset, double y_offset) override;
};

void application::scene_init()
{
	renderer = new dodo::renderer;
	renderer->set_clear_color({0, 0, .2, 1});

	// build a custom shader program from the source code at the top of this file
	custom_shader = new dodo::shader::shader_program(
		std::make_shared<dodo::shader::shader>(dodo::shader::vertex_shader, vertex_shader_code, dodo::shader::source_code),
		std::make_shared<dodo::shader::shader>(dodo::shader::fragment_shader, fragment_shader_code, dodo::shader::source_code)
	);
	if (!custom_shader->ready())
	{
		std::cerr << "Failed to compile shader: " << custom_shader->error_msg() << std::endl;
		error_ = true; return;
	}
	else
	{
		// we can print the shader's debug info to see that the custom uniform is properly registered
		std::cout << "Successfully compiled shader." << std::endl;
		custom_shader->print_debug_info(std::cout);
	}

	camera = std::make_shared<dodo::perspective_camera>();
	camera_controller = std::make_shared<dodo::arcball_camera_controller>(camera);
	camera_controller->set_position({0, 0, 0});
	renderer->set_camera(camera);

	auto material = dodo::materials::standard();
	material.texture = std::make_shared<dodo::texture>(dodo::texture::from_image_file(ASSET_PATH "textures/tiles_diffuse.png"));
	sphere = new dodo::model(new dodo::standard_mesh(dodo::standard_mesh::generate_sphere(3, 24, 12)), {0,0,0}, material);
}

void application::scene_destroy()
{
	delete renderer;
	delete custom_shader;
	delete sphere;
}

void application::update()
{
	renderer->clear();

	// we can retrieve the location of the custom uniform by its' name and set its' value every frame
	// the uniform location does not change after shader compilation, so this example may be excessive
	// it is still fairly efficient, as the below code only causes a hashmap lookup and does not involve OpenGL
	// for fast lookups, the templated uniform_location hashes the uniform name at compile time
	auto uniform_location = custom_shader->uniform_location<"custom_color">();
	custom_shader->set_uniform(uniform_location, custom_color);

	renderer->render_model(*sphere, *custom_shader);
}

void application::on_key(GLFWwindow* window, int key, int scan_code, int action, int mods)
{
	std::uniform_real_distribution<float> dist(0, 1);
	static std::default_random_engine rng;
	if (action == GLFW_PRESS && key == GLFW_KEY_SPACE)
		custom_color = { dist(rng), dist(rng), dist(rng) };
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
		camera_controller->yaw(rotate_angle * static_cast<float>(mouse.x - x_pos));
		camera_controller->pitch(rotate_angle * -static_cast<float>(mouse.y - y_pos));
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

void application::on_scroll(GLFWwindow* window, double x_offset, double y_offset)
{
	const float zoom_distance = -2.f * y_offset;
	camera_controller->set_distance(camera_controller->distance() + zoom_distance);
}

int main()
{
	application app;
	app.run();
	return 0;
}
