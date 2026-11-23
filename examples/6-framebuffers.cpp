/**
 * @file framebuffers.cpp
 *
 * An example showcasing how to use framebuffers in Dodo.
 * A plane is rendered in the center of the scene, and its texture is the color attachment of a framebuffer.
 * The contents of the default framebuffer (the screen) are copied to the framebuffer of the plane every frame.
 * A textured sphere is rendered in front of the plane, and a cubemap skybox is rendered around it.
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
#include <Framebuffer.h>

#define ASSET_PATH "examples/"

class application;

class application : public dodo::application
{
	dodo::renderer* renderer = nullptr;
	dodo::shader::shader_program* shader;
	dodo::shader::shader_program* skybox_shader;
	std::shared_ptr<dodo::perspective_camera> camera;
	dodo::model* plane;
	dodo::model* sphere;
	std::shared_ptr<dodo::texture> skybox;

	dodo::framebuffer::framebuffer<dodo::texture>* plane_buffer;

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

	shader = dodo::default_shaders::textured_mesh();
	skybox_shader = dodo::default_shaders::skybox();

	camera = std::make_shared<dodo::perspective_camera>();
	camera->set_position({-1.617142f, 2.48f, 2.136936f});
	camera->set_yaw(-0.61f);
	camera->set_pitch(-0.56f);
	renderer->set_camera(camera);

	// create a framebuffer with a color texture attachment and no depth attachment
	plane_buffer = new dodo::framebuffer::framebuffer<dodo::texture>(800, 800);

	// create a plane whose texture is the color attachment of the framebuffer
	auto plane_material = dodo::materials::standard();
	plane_material.texture = plane_buffer->color_ptr();
	plane = new dodo::model(new dodo::standard_mesh(
		dodo::standard_mesh::generate_2d_rect(5, 5)),
		{0,1,5}, plane_material, {0, glm::pi<float>(), 0}
	);

	auto sphere_material = dodo::materials::standard();
	sphere_material.texture = std::make_shared<dodo::texture>(dodo::texture::from_image_file(ASSET_PATH "textures/tiles_diffuse.png"));
	sphere = new dodo::model(
		new dodo::standard_mesh(dodo::standard_mesh::generate_sphere(1, 24, 12)), {0,0,4}, sphere_material
	);

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
	delete plane;
	delete sphere;
	delete plane_buffer;
}

void application::handle_movement_input()
{
	float delta = 0.16f;
	if (glfwGetKey(window_, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		delta *= 2;
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

	// disable backface culling for the plane so that we can see it from both sides
	renderer->next_call(NULL, dodo::renderer::render_flags::backface_culling);
	renderer->render_model(*plane, *shader);
	renderer->render_model(*sphere, *shader);
	sphere->rotate({.01f, .02f, .01f});
	renderer->render_skybox(*skybox, *skybox_shader);

	// copy the contents of the default framebuffer which we just rendered to the framebuffer of the plane
	dodo::framebuffer::get_default().blit_to(*plane_buffer);
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
