#include "Egg.h"

#include "DefaultShaders.h"

static egg::application* app;

void egg::application::handle_wasd_movement()
{
	float delta = 0.16f;
	if (glfwGetKey(window_, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		delta *= 2;
	else if (glfwGetKey(window_, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		delta *= 0.5f;
	glm::vec3 forward_vec = default_camera_controller->view_vector();
	forward_vec.y = 0;
	forward_vec = normalize(forward_vec) * delta;
	glm::vec3 lateral_vec = glm::normalize(glm::cross(forward_vec, {0, 1, 0})) * delta;
	if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS)
		default_camera_controller->translate(forward_vec);
	if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS)
		default_camera_controller->translate(-forward_vec);
	if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS)
		default_camera_controller->translate(-lateral_vec);
	if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS)
		default_camera_controller->translate(lateral_vec);
	if (glfwGetKey(window_, GLFW_KEY_R) == GLFW_PRESS)
		default_camera_controller->translate({0, delta, 0});
	if (glfwGetKey(window_, GLFW_KEY_F) == GLFW_PRESS)
		default_camera_controller->translate({0, -delta, 0});
}

egg::application::application(const char* title, int width, int height, bool resizeable)
	: dodo::application()
{
	settings.title = title;
	settings.width = width;
	settings.height = height;
	settings.resizeable = resizeable;
	::app = this;
}

void egg::application::setup()
{
	if (!glfwInit())
	{
		error_ = true;
		return;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	glfwWindowHint(GLFW_REFRESH_RATE, 0);
	glfwWindowHint(GLFW_RESIZABLE,  settings.resizeable ? GLFW_TRUE : GLFW_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4); // enable anti-aliasing

	window_ = glfwCreateWindow(settings.width, settings.height, settings.title.c_str(), nullptr, nullptr);
	if (!window_)
	{
		error_ = true;
		return;
	}
	glfwMakeContextCurrent(window_);
	gladLoadGL();

	if (glDebugMessageCallback != nullptr)
	{
		glDebugMessageCallback(debug_callback, nullptr);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}

	glViewport(0, 0, settings.width, settings.height);
}

void egg::application::scene_init()
{
	default_renderer = new dodo::renderer;
	default_renderer->set_clear_color({0, 0, .1, 1});
	default_renderer->set_flags(dodo::renderer::render_flags::wireframe, dodo::renderer::render_flags::backface_culling);

	default_shader = dodo::default_shaders::basic();
	default_skybox_shader = dodo::default_shaders::skybox();

	default_camera = std::make_shared<dodo::perspective_camera>(
		glm::vec3{0, 0, 0}, glm::vec3{0, 1, 0},
		settings.fov, settings.near, settings.far,
		static_cast<float>(settings.width) / settings.height
	);
	default_camera_controller = std::make_shared<dodo::arcball_camera_controller>(default_camera);
	default_camera_controller->set_position({0, 0, 0});

	default_sphere = new dodo::model(new dodo::standard_mesh(dodo::standard_mesh::generate_sphere(1.f, 32, 16)), {0, 0, 0}, dodo::materials::standard{});

	default_renderer->set_camera(default_camera);
}

void egg::application::scene_destroy()
{
	delete default_renderer;
	delete default_shader;
	delete default_skybox_shader;
	delete default_sphere;
}

void egg::application::update()
{
	handle_wasd_movement();
	default_renderer->clear();
	if (default_skybox)
	{
		default_renderer->next_call(NULL, dodo::renderer::render_flags::wireframe);
		default_renderer->render_skybox(*default_skybox, *default_skybox_shader);
	}
}

void egg::application::on_cursor_pos(GLFWwindow* window, double x_pos, double y_pos)
{
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS)
	{
		app->mouse.pressed = false;
		return;
	}
	if (app->mouse.pressed)
	{
		const float rotate_angle = -0.005f;
		app->default_camera_controller->yaw(rotate_angle * static_cast<float>(app->mouse.x - x_pos));
		app->default_camera_controller->pitch(rotate_angle * -static_cast<float>(app->mouse.y - y_pos));
		app->mouse.x = x_pos;
		app->mouse.y = y_pos;
	}
	else
	{
		app->mouse.x = x_pos;
		app->mouse.y = y_pos;
		app->mouse.pressed = true;
	}
}

void egg::application::on_scroll(GLFWwindow* window, double x_offset, double y_offset)
{
	const float zoom_distance = -2.f * y_offset;
	app->default_camera_controller->set_distance(app->default_camera_controller->distance() + zoom_distance);
}

void egg::application::on_framebuffer_size_change(GLFWwindow* window, int width, int height)
{
	if (height == 0 || width == 0)
		return; // ignore minimize
	glViewport(0, 0, width, height);
	default_camera->set_aspect_ratio(width, height);
}
