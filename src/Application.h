#pragma once


#include "Dodo.h"

namespace dodo
{
	class application
	{
	protected:
		static inline application* callback_target_;

		GLFWwindow* window_ = nullptr;
		bool error_ = false;

	public:
		application() = default;
		virtual ~application() = default;

		void run();

	protected:
		virtual void setup();
		virtual void update() = 0;
		virtual void scene_init() = 0;
		virtual void scene_destroy() = 0;
		virtual void destroy();

		virtual void on_error(int error_code, const char* error_message) {}
		virtual void on_monitor_event(GLFWmonitor* monitor, int event) {}
		virtual void on_window_pos_change(GLFWwindow* window, int x_pos, int y_pos) {}
		virtual void on_window_size_change(GLFWwindow* window, int width, int height) {}
		virtual void on_window_close(GLFWwindow* window) {}
		virtual void on_window_refresh(GLFWwindow* window) {}
		virtual void on_window_focus(GLFWwindow* window, int focused) {}
		virtual void on_window_iconify(GLFWwindow* window, int iconified) {}
		virtual void on_window_maximize(GLFWwindow* window, int maximized) {}
		virtual void on_framebuffer_size_change(GLFWwindow* window, int width, int height);
		virtual void on_window_content_scale_change(GLFWwindow* window, float x_scale, float y_scale) {}
		virtual void on_key(GLFWwindow* window, int key, int scan_code, int action, int mods);
		virtual void on_char(GLFWwindow* window, unsigned int code_point) {}
		virtual void on_char_mods(GLFWwindow* window, unsigned int code_point, int mods) {}
		virtual void on_mouse_button(GLFWwindow* window, int button, int action, int mods) {}
		virtual void on_cursor_pos(GLFWwindow* window, double x_pos, double y_pos) {}
		virtual void on_cursor_enter(GLFWwindow* window, int entered) {}
		virtual void on_scroll(GLFWwindow* window, double x_offset, double y_offset) {}
		virtual void on_drop(GLFWwindow* window, int count, const char** paths) {}
		virtual void on_joystick(int joystick_id, int event) {}

		virtual void set_callbacks();

		static void APIENTRY debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
		                                    const GLchar* message, const GLvoid* user_param);
	};

}
