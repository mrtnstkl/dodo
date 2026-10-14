#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>

#include <Application.h>
#include <Renderer.h>
#include <Shader.h>
#include <Texture.h>
#include <Model.h>
#include <Camera.h>
#include <Mesh.h>
#include <Instanced.h>
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

	std::list<std::shared_ptr<dodo::point_light>> point_lights;
	std::list<std::shared_ptr<dodo::directional_light>> directional_lights;

	dodo::model* sphere;

	std::shared_ptr<dodo::texture> skybox;

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
	camera = std::make_shared<dodo::perspective_camera>();
	renderer->set_camera(camera);
	camera_controller = std::make_shared<dodo::arcball_camera_controller>(camera);
	camera_controller->set_position({});

	// read the phong shader from disk and compile it
	shader = new dodo::shader::shader_program(ASSET_PATH "shaders/phong.vert", ASSET_PATH "shaders/phong_reflective.frag");
	skybox_shader = dodo::default_shaders::skybox();

	skybox = std::make_shared<dodo::texture>(dodo::texture::cubemap_from_image_files(
		ASSET_PATH "textures/cubemaps/posX.png", ASSET_PATH "textures/cubemaps/posY.png", ASSET_PATH "textures/cubemaps/posZ.png",
		ASSET_PATH "textures/cubemaps/negX.png", ASSET_PATH "textures/cubemaps/negY.png", ASSET_PATH "textures/cubemaps/negZ.png"
	));

	auto sphere_material = dodo::materials::standard{
		{1,1,1}, 0.1f, 0.7f, 1.0f, 8,
		std::make_shared<dodo::texture>(dodo::texture::from_image_file(ASSET_PATH "textures/tiles_diffuse.png")),
		std::make_shared<dodo::texture>(dodo::texture::from_image_file(ASSET_PATH "textures/tiles_specular.png")),
		skybox
	};
	sphere = new dodo::model(new dodo::standard_mesh(dodo::standard_mesh::generate_sphere(3, 32, 16)), {0,0,0}, sphere_material);

	// create 4 point lights with different colors and positions, and one directional light
	const auto attenuation = glm::vec3{ 1, 0.4f, 0.1f };
	point_lights.push_back(std::make_shared<dodo::point_light>(glm::vec3{ -2,3,-3 }, glm::vec3{ 1,0,1 }, attenuation));
	point_lights.push_back(std::make_shared<dodo::point_light>(glm::vec3{ -2,3,3 }, glm::vec3{ 0,1,0 }, attenuation));
	point_lights.push_back(std::make_shared<dodo::point_light>(glm::vec3{ -2,-3,-3 }, glm::vec3{ 0,0,1 }, attenuation));
	point_lights.push_back(std::make_shared<dodo::point_light>(glm::vec3{ -2,-3,3 }, glm::vec3{ 1,1,0 }, attenuation));
	directional_lights.push_back(std::make_shared<dodo::directional_light>(glm::vec3{ 1, .3, 0 }, glm::vec3{ 1,1,1 }));

	// add the lights to the renderer
	for (auto& point_light : point_lights)
		renderer->add_point_light(point_light);
	for (auto& directional_light : directional_lights)
		renderer->add_directional_light(directional_light);
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
	sphere->rotate({0, .003, 0});
	renderer->render_skybox(*skybox, *skybox_shader);
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
