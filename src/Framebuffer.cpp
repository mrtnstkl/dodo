#include "Framebuffer.h"
#include <memory>
#include "Dodo.h"
#include "GLFW/glfw3.h"

void dodo::framebuffer::renderbuffer::init()
{
	glBindRenderbuffer(GL_RENDERBUFFER, handle_);
	glRenderbufferStorage(GL_RENDERBUFFER, format_, width_, height_);
	glBindRenderbuffer(GL_RENDERBUFFER, NULL);
}

dodo::framebuffer::renderbuffer::renderbuffer(uint width, uint height, GLenum format) : format_(format), height_(height), width_(width)
{
	glGenRenderbuffers(1, &handle_);
	init();
}

dodo::framebuffer::renderbuffer::renderbuffer(renderbuffer&& other) noexcept
	: handle_(other.handle_),
	  format_(other.format_),
	  height_(other.height_),
	  width_(other.width_)
{
	other.handle_ = NULL;
}

dodo::framebuffer::renderbuffer& dodo::framebuffer::renderbuffer::operator=(renderbuffer&& other) noexcept
{
	if (this == &other)
		return *this;
	handle_ = other.handle_;
	format_ = other.format_;
	height_ = other.height_;
	width_ = other.width_;

	other.handle_ = NULL;

	return *this;
}

dodo::framebuffer::renderbuffer::~renderbuffer()
{
	glDeleteRenderbuffers(1, &handle_);
}

GLuint dodo::framebuffer::renderbuffer::handle() const
{
	return handle_;
}

void dodo::framebuffer::renderbuffer::bind() const
{
	glBindRenderbuffer(GL_RENDERBUFFER, handle_);
}

void dodo::framebuffer::framebuffer_base::bind(GLenum target) const
{
	glBindFramebuffer(target, handle());
}

void dodo::framebuffer::framebuffer_base::blit_to(dodo::framebuffer::framebuffer_base& other, GLbitfield buffer)
{
	GLint draw_bak = 0, read_bak = 0;
	glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read_bak);
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw_bak);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, handle());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, other.handle());

	glBlitFramebuffer(0, 0, width(), height(), 0, 0, other.width(), other.height(), buffer, GL_NEAREST);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, read_bak);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw_bak);
}

void dodo::framebuffer::bind_default(GLenum target)
{
	default_frambuffer::instance().bind(target);
}

dodo::framebuffer::default_frambuffer::default_frambuffer(GLFWwindow* window)
	: window_(window)
{
}

std::shared_ptr<dodo::framebuffer::default_frambuffer> dodo::framebuffer::default_frambuffer::instance_ptr()
{
	static thread_local auto instance = std::shared_ptr<default_frambuffer>(new default_frambuffer(glfwGetCurrentContext()));
	return instance;
}

dodo::framebuffer::default_frambuffer& dodo::framebuffer::default_frambuffer::instance()
{
	static thread_local auto instance = instance_ptr();
	return *instance.get();
}

GLuint dodo::framebuffer::default_frambuffer::handle() const
{
	return 0;
}

uint dodo::framebuffer::default_frambuffer::width() const
{
	int width, height;
	glfwGetFramebufferSize(window_, &width, &height);
	return static_cast<uint>(width);
}

uint dodo::framebuffer::default_frambuffer::height() const
{
	int width, height;
	glfwGetFramebufferSize(window_, &width, &height);
	return static_cast<uint>(height);
}
