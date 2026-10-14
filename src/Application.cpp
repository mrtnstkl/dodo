#include "Application.h"

#include <iostream>

#include "Util.h"

void dodo::application::run()
{
	setup();
	if (error_) return;

	scene_init();
	if(error_) return;

	set_callbacks();
	if (error_) return;

	while (!glfwWindowShouldClose(window_))
	{
		update();
		if (error_) return;
		glfwSwapBuffers(window_);
		glfwPollEvents();
	}

	scene_destroy();
}

void dodo::application::setup()
{
	// default implementation
	// you probably want to override this function

	constexpr auto width = 800;
	constexpr auto height = 800;
	constexpr auto title = "Demo Window";

	//  init glfw
	if (!glfwInit())
	{
		error_ = true;
		return;
	}

	// set opengl version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	glfwWindowHint(GLFW_REFRESH_RATE, 0);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4); // enable anti-aliasing

	// create window
	window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (!window_)
	{
		error_ = true;
		return;
	}
	glfwMakeContextCurrent(window_);

	// init glew
	gladLoadGL();

	// set opengl debug callback and enable debug output
	if (glDebugMessageCallback != nullptr)
	{
		glDebugMessageCallback(debug_callback, nullptr);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}

	glViewport(0, 0, width, height);
}

void dodo::application::destroy()
{
	glfwTerminate();
}

void dodo::application::on_key(GLFWwindow* window, int key, int scan_code, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
}

void dodo::application::on_framebuffer_size_change(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void dodo::application::set_callbacks()
{
	callback_target_ = this;

	glfwSetErrorCallback([](int error_code, const char* error_message)
	{
		callback_target_->on_error(error_code, error_message);
	});
	glfwSetMonitorCallback([](GLFWmonitor* monitor, int event)
	{
		callback_target_->on_monitor_event(monitor, event);
	});
	glfwSetWindowPosCallback(window_, [](GLFWwindow* window, int x_pos, int y_pos)
	{
		callback_target_->on_window_pos_change(window, x_pos, y_pos);
	});
	glfwSetWindowSizeCallback(window_, [](GLFWwindow* window, int width, int height)
	{
		callback_target_->on_window_size_change(window, width, height);
	});
	glfwSetWindowCloseCallback(window_, [](GLFWwindow* window)
	{
		callback_target_->on_window_close(window);
	});
	glfwSetWindowRefreshCallback(window_, [](GLFWwindow* window)
	{
		callback_target_->on_window_refresh(window);
	});
	glfwSetWindowFocusCallback(window_, [](GLFWwindow* window, int focused)
	{
		callback_target_->on_window_focus(window, focused);
	});
	glfwSetWindowIconifyCallback(window_, [](GLFWwindow* window, int iconified)
	{
		callback_target_->on_window_iconify(window, iconified);
	});
	glfwSetWindowMaximizeCallback(window_, [](GLFWwindow* window, int maximized)
	{
		callback_target_->on_window_maximize(window, maximized);
	});
	glfwSetFramebufferSizeCallback(window_, [](GLFWwindow* window, int width, int height)
	{
		callback_target_->on_framebuffer_size_change(window, width, height);
	});
	glfwSetWindowContentScaleCallback(window_, [](GLFWwindow* window, float x_scale, float y_scale)
	{
		callback_target_->on_window_content_scale_change(window, x_scale, y_scale);
	});
	glfwSetKeyCallback(window_, [](GLFWwindow* window, int key, int scan_code, int action, int mods)
	{
		callback_target_->on_key(window, key, scan_code, action, mods);
	});
	glfwSetCharCallback(window_, [](GLFWwindow* window, unsigned int code_point)
	{
		callback_target_->on_char(window, code_point);
	});
	glfwSetCharModsCallback(window_, [](GLFWwindow* window, unsigned int code_point, int mods)
	{
		callback_target_->on_char_mods(window, code_point, mods);
	});
	glfwSetMouseButtonCallback(window_, [](GLFWwindow* window, int button, int action, int mods)
	{
		callback_target_->on_mouse_button(window, button, action, mods);
	});
	glfwSetCursorPosCallback(window_, [](GLFWwindow* window, double x_pos, double y_pos)
	{
		callback_target_->on_cursor_pos(window, x_pos, y_pos);
	});
	glfwSetCursorEnterCallback(window_, [](GLFWwindow* window, int entered)
	{
		callback_target_->on_cursor_enter(window, entered);
	});
	glfwSetScrollCallback(window_, [](GLFWwindow* window, double x_offset, double y_offset)
	{
		callback_target_->on_scroll(window, x_offset, y_offset);
	});
	glfwSetDropCallback(window_, [](GLFWwindow* window, int count, const char** paths)
	{
		callback_target_->on_drop(window, count, paths);
	});
	glfwSetJoystickCallback([](int joystick_id, int event)
	{
		callback_target_->on_joystick(joystick_id, event);
	});
}

void dodo::application::debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                      const GLchar* message, const GLvoid* user_param)
{
	// ignore performance warnings (buffer uses GPU memory, shader recompilation) from nvidia
	if (id == 131185 || id == 131218)
	{
		return;
	}
	std::cout << util::format_gl_debug_output(source, type, id, severity, message) << std::endl;
}
