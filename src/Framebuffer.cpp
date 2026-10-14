#include "Framebuffer.h"

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

void dodo::framebuffer::bind_default(GLenum target)
{
	glBindFramebuffer(target, 0);
}
