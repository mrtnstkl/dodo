#pragma once

#include <memory>
#include "Application.h"
#include "Camera.h"
#include "Model.h"
#include "Renderer.h"

namespace egg
{
	class application : public dodo::application
	{
	protected:
		struct
		{
			double x = 0, y = 0;
			bool pressed = false;
		} mouse;
		struct
		{
			float fov = 75.f;
			float near = 0.1f;
			float far = 100.f;
			int width;
			int height;
			std::string title;
			bool resizeable;
		} settings;

		dodo::renderer *default_renderer = nullptr;
		std::shared_ptr<dodo::perspective_camera> default_camera;
		std::shared_ptr<dodo::arcball_camera_controller> default_camera_controller;
		dodo::shader::shader_program *default_shader;
		dodo::shader::shader_program *default_skybox_shader;
		std::shared_ptr<dodo::texture> default_skybox;
		dodo::model *default_sphere;

		void handle_wasd_movement();

	public:
		application(const char *title = "Egg", int width = 800, int height = 600, bool resizeable = true);

		void setup() override;

		void scene_init() override;
		void scene_destroy() override;

		void update() override;

		void on_cursor_pos(GLFWwindow *window, double x_pos, double y_pos) override;
		void on_scroll(GLFWwindow *window, double x_offset, double y_offset) override;
		void on_framebuffer_size_change(GLFWwindow *window, int width, int height) override;
	};

} // namespace egg
