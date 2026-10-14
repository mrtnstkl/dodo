/**
 * @file hello.cpp
 *
 * A minimal example showing how to instantiate the application and render a simple mesh.
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


class application : public dodo::application
{
	dodo::renderer* renderer = nullptr;
	dodo::shader::shader_program* shader;
	std::shared_ptr<dodo::perspective_camera> camera;
	dodo::model* sphere;

	void scene_init() override;
	void scene_destroy() override;

	void update() override;
};

void application::scene_init()
{
	renderer = new dodo::renderer;
	renderer->set_clear_color({0, 0, .2, 1});
	renderer->set_flags(dodo::renderer::render_flags::wireframe);

	shader = dodo::default_shaders::basic();

	camera = std::make_shared<dodo::perspective_camera>();
	renderer->set_camera(camera);

	sphere = new dodo::model(new dodo::standard_mesh(dodo::standard_mesh::generate_sphere(1, 8, 5)), {0, 0, 3});
}

void application::scene_destroy()
{
	delete renderer;
	delete shader;
	delete sphere;
}

void application::update()
{
	sphere->rotate({.01, .02, .01});
	renderer->clear();
	renderer->render_model(*sphere, *shader);
}

int main()
{
	application app;
	app.run();
	return 0;
}
