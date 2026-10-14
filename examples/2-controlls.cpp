/**
 * @file controlls.cpp
 *
 * An example showcasing simple camera controlls with an arcball camera.
 * WASD keys move the orbit point of the camera.
 * Left mouse button rotates the camera around the orbit point.
 * Scroll wheel zooms the camera in and out.
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

class application;

class application : public dodo::application
{
	dodo::renderer* renderer = nullptr;
	dodo::shader::shader_program* shader;
	std::shared_ptr<dodo::perspective_camera> camera;
	std::shared_ptr<dodo::arcball_camera_controller> camera_controller;
	dodo::model* sphere;
	dodo::model* cylinder;

	struct {
		double x = 0, y = 0;
		bool pressed = false;
	} mouse;

	void scene_init() override;
	void scene_destroy() override;

	void update() override;

	void on_cursor_pos(GLFWwindow* window, double x_pos, double y_pos) override;
	void on_scroll(GLFWwindow* window, double x_offset, double y_offset) override;
};

void application::scene_init()
{
	renderer = new dodo::renderer;
	renderer->set_clear_color({0, 0, .2, 1});
	renderer->set_flags(dodo::renderer::render_flags::wireframe);

	shader = dodo::default_shaders::basic();

	camera = std::make_shared<dodo::perspective_camera>();
	camera_controller = std::make_shared<dodo::arcball_camera_controller>(camera);
	camera_controller->set_position({0, 0, 0});
	renderer->set_camera(camera);

	sphere = new dodo::model(new dodo::standard_mesh(dodo::standard_mesh::generate_sphere(1, 8, 5)));
	cylinder = new dodo::model(new dodo::standard_mesh(dodo::standard_mesh::generate_cylinder(0.2, 0.2, 8)), {0, 0, 0});
}

void application::scene_destroy()
{
	delete renderer;
	delete shader;
	delete sphere;
	delete cylinder;
}

void application::update()
{
	// simple camera movement with WASD
	float delta = 0.16f;
	if (glfwGetKey(window_, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		delta *= 2;
	else if (glfwGetKey(window_, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		delta *= 0.5f;
	glm::vec3 forward_vec = camera_controller->view_vector();
	forward_vec.y = 0;
	forward_vec = normalize(forward_vec) * delta;
	glm::vec3 lateral_vec = glm::normalize(glm::cross(forward_vec, {0, 1, 0})) * delta;
	if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS)
		camera_controller->translate(forward_vec);
	if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS)
		camera_controller->translate(-forward_vec);
	if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS)
		camera_controller->translate(-lateral_vec);
	if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS)
		camera_controller->translate(lateral_vec);

	renderer->clear();
	// render cyclinder at the orbit center of the camera
	cylinder->set_position(camera_controller->position());
	renderer->render_model(*cylinder, *shader);
	// render 4 spheres
	sphere->set_position({0, 0, 2});
	renderer->render_model(*sphere, *shader);
	sphere->set_position({0, 0, -2});
	renderer->render_model(*sphere, *shader);
	sphere->set_position({2, 0, 0});
	renderer->render_model(*sphere, *shader);
	sphere->set_position({-2, 0, 0});
	renderer->render_model(*sphere, *shader);
}

void application::on_cursor_pos(GLFWwindow* window, double x_pos, double y_pos)
{
	// simple orbit camera controlls with mouse
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
	// scroll wheel zooms the camera in and out
	const float zoom_distance = -2.f * y_offset;
	camera_controller->set_distance(camera_controller->distance() + zoom_distance);
}

int main()
{
	application app;
	app.run();
	return 0;
}
