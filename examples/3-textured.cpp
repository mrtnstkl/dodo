/**
 * @file textured.cpp
 *
 * An example showcasing how to render textured models and skyboxes.
 * A textured sphere is rendered in the center of the scene, and a cubemap skybox is rendered around it.
 * F1 toggles wireframe mode, F2 toggles backface culling.
 * Left mouse button rotates the camera around the center of the scene, scroll wheel zooms in and out.
 *
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>

#include <Application.h>
#include <Renderer.h>
#include <Shader.h>
#include <Model.h>
#include <Camera.h>
#include <Mesh.h>
#include <DefaultShaders.h>

#define ASSET_PATH "examples/"

class application;

class application : public dodo::application
{
	dodo::renderer* renderer = nullptr;
	dodo::shader::shader_program* shader;
	dodo::shader::shader_program* skybox_shader;
	std::shared_ptr<dodo::perspective_camera> camera;
	std::shared_ptr<dodo::arcball_camera_controller> camera_controller;
	dodo::model* sphere;
	std::shared_ptr<dodo::texture> skybox;

	struct {
		double x = 0, y = 0;
		bool pressed = false;
	} mouse;

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

	shader = dodo::default_shaders::textured_mesh();
	skybox_shader = dodo::default_shaders::skybox();

	camera = std::make_shared<dodo::perspective_camera>();
	camera_controller = std::make_shared<dodo::arcball_camera_controller>(camera);
	camera_controller->set_position({0, 0, 0});
	renderer->set_camera(camera);

	// create a textured sphere model
	// the texture is supplied to the model through the material
	auto material = dodo::materials::standard();
	material.texture = std::make_shared<dodo::texture>(dodo::texture::from_image_file(ASSET_PATH "textures/tiles_diffuse.png"));
	sphere = new dodo::model(new dodo::standard_mesh(dodo::standard_mesh::generate_sphere(3, 24, 12)), {0,0,0}, material);

	// create a cubemap texture for the skybox
	skybox = std::make_shared<dodo::texture>(dodo::texture::cubemap_from_image_files(
		ASSET_PATH "textures/cubemaps/posX.png", ASSET_PATH "textures/cubemaps/posY.png", ASSET_PATH "textures/cubemaps/posZ.png",
		ASSET_PATH "textures/cubemaps/negX.png", ASSET_PATH "textures/cubemaps/negY.png", ASSET_PATH "textures/cubemaps/negZ.png"
	));
}

void application::scene_destroy()
{
	delete renderer;
	delete shader;
	delete skybox_shader;
	delete sphere;
}

void application::update()
{
	renderer->clear();
	renderer->render_model(*sphere, *shader);
	// we can use next_call to override render flags for the next draw call without affecting the general render state
	// here we explicitly disable wireframe when rendering the skybox
	renderer->next_call(NULL, dodo::renderer::render_flags::wireframe);
	renderer->render_skybox(*skybox, *skybox_shader);
}

void application::on_key(GLFWwindow* window, int key, int scan_code, int action, int mods)
{
	if (action != GLFW_PRESS)
		return;
	switch (key)
	{
	case GLFW_KEY_F1:
		// toggle wireframe mode with F1
		renderer->set_flags(~renderer->flags(), dodo::renderer::render_flags::wireframe);
		break;
	case GLFW_KEY_F2:
		// toggle backface culling with F2
		renderer->set_flags(~renderer->flags(), dodo::renderer::render_flags::backface_culling);
		break;
	}
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
