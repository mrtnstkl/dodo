#pragma once

#include "Dodo.h"
#include "Texture.h"

typedef unsigned int uint;


namespace dodo::framebuffer
{

	class renderbuffer
	{
		GLuint handle_ = NULL;
		GLenum format_;

		uint height_;
		uint width_;

		void init();

	public:
		renderbuffer(uint width, uint height, GLenum format);

		renderbuffer(const renderbuffer& other) = delete;

		renderbuffer(renderbuffer&& other) noexcept;

		renderbuffer& operator=(const renderbuffer& other) = delete;

		renderbuffer& operator=(renderbuffer&& other) noexcept;

		~renderbuffer();

		[[nodiscard]] GLuint handle() const;

		void bind() const;
	};

	class none;


	template <typename C = none, typename D = none>
	class framebuffer
	{
		static_assert(std::is_same_v<C, texture> || std::is_same_v<C, renderbuffer> || std::is_same_v<C, none>,
			"C must be of type texture, renderbuffer, or none");
		static_assert(std::is_same_v<D, texture> || std::is_same_v<D, renderbuffer> || std::is_same_v<D, none>,
			"D must be of type texture, renderbuffer, or none");

		GLuint framebuffer_handle_;

		uint width_, height_;

		GLenum color_format_;
		GLenum depth_format_;
		GLenum color_type_;
		GLenum depth_type_;

		std::shared_ptr<C> color_;
		std::shared_ptr<D> depth_;

		friend class framebuffer<none, none>;
		friend class framebuffer<none, texture>;
		friend class framebuffer<none, renderbuffer>;
		friend class framebuffer<texture, none>;
		friend class framebuffer<renderbuffer, none>;

	public:
		framebuffer(uint width, uint height,
		            GLenum color_format = GL_RGB, GLenum depth_format = GL_DEPTH_COMPONENT,
		            GLenum color_type = GL_UNSIGNED_BYTE, GLenum depth_type = GL_FLOAT);

		framebuffer(const framebuffer& other) = delete;

		framebuffer(framebuffer&& other) noexcept;

		framebuffer& operator=(const framebuffer& other) = delete;

		framebuffer& operator=(framebuffer&& other) noexcept;

		~framebuffer();


		const C& color() const;

		/**
		 * \brief	Returns a shared pointer to the color buffer.
		 */
		std::shared_ptr<C> color_ptr();

		const D& depth() const;

		/**
		 * \brief	Returns a shared pointer to the depth buffer.
		 */
		std::shared_ptr<D> depth_ptr();

		uint width() const;
		uint height() const;

		/**
		 * \brief	Binds the framebuffer.
		 */
		void bind(GLenum target = GL_FRAMEBUFFER) const;

		template <typename F>
		void blit_to(F& other, GLbitfield buffer = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		void blit_to_default(GLbitfield buffer = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) const;

		void blit_from_default(GLbitfield buffer = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) const;
	};

	using framebuffer_rr = framebuffer<renderbuffer, renderbuffer>;
	using framebuffer_tr = framebuffer<texture, renderbuffer>;
	using framebuffer_rt = framebuffer<renderbuffer, texture>;
	using framebuffer_tt = framebuffer<texture, texture>;

	class none
	{
	private:
		explicit none() = default;

		friend class framebuffer<none, none>;
		friend class framebuffer<none, texture>;
		friend class framebuffer<none, renderbuffer>;
		friend class framebuffer<texture, none>;
		friend class framebuffer<renderbuffer, none>;
	};

	void bind_default(GLenum target = GL_FRAMEBUFFER);


	template <typename C, typename D>
	framebuffer<C, D>::framebuffer(uint width, uint height, GLenum color_format, GLenum depth_format, GLenum color_type,
	                               GLenum depth_type)
		: width_(width), height_(height),
		  color_format_(color_format), depth_format_(depth_format),
		  color_type_(color_type), depth_type_(depth_type),
		  color_([&]
		  {
			  if constexpr (std::is_same_v<C, texture>)
				  return std::make_shared<texture>(texture::create_tex_image_2d(GL_TEXTURE_2D, color_format, width, height, color_format, color_type));
			  else if constexpr (std::is_same_v<C, renderbuffer>)
				  return std::make_shared<renderbuffer>(width, height, color_format);
			  else
				  return std::shared_ptr<none>(new none);
		  }()),
		  depth_([&]
		  {
			  if constexpr (std::is_same_v<D, texture>)
				  return std::make_shared<texture>(texture::create_tex_image_2d(GL_TEXTURE_2D, depth_format, width, height, depth_format, depth_type));
			  else if constexpr (std::is_same_v<D, renderbuffer>)
				  return std::make_shared<renderbuffer>(width, height, depth_format);
			  else
				  return std::shared_ptr<none>(new none);
		  }())
	{
		glGenFramebuffers(1, &framebuffer_handle_);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_handle_);

		const GLenum depth_attachment = (depth_type == GL_DEPTH24_STENCIL8 || depth_type == GL_DEPTH32F_STENCIL8
			                                 ? GL_DEPTH_STENCIL_ATTACHMENT
			                                 : GL_DEPTH_ATTACHMENT);

		if constexpr (std::is_same_v<C, texture>)
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_->handle(), 0);
		else if constexpr (std::is_same_v<C, renderbuffer>)
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color_->handle());
		else
		{
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}

		if constexpr (std::is_same_v<D, texture>)
			glFramebufferTexture2D(GL_FRAMEBUFFER, depth_attachment, GL_TEXTURE_2D, depth_->handle(), 0);
		else if constexpr (std::is_same_v<D, renderbuffer>)
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, depth_attachment, GL_RENDERBUFFER, depth_->handle());


		glBindFramebuffer(GL_FRAMEBUFFER, NULL);
	}

	template <typename C, typename D>
	framebuffer<C, D>::framebuffer(framebuffer&& other) noexcept
		: framebuffer_handle_(other.framebuffer_handle_),
		  width_(other.width_),
		  height_(other.height_),
		  color_format_(other.color_format_),
		  depth_format_(other.depth_format_),
		  color_type_(other.color_type_),
		  depth_type_(other.depth_type_),
		  color_(std::move(other.color_)),
		  depth_(std::move(other.depth_))
	{
		other.framebuffer_handle_ = NULL;
	}

	template <typename C, typename D>
	framebuffer<C, D>& framebuffer<C, D>::operator=(framebuffer&& other) noexcept
	{
		if (this == &other)
			return *this;
		glDeleteFramebuffers(1, &framebuffer_handle_);
		framebuffer_handle_ = other.framebuffer_handle_;
		width_ = other.width_;
		height_ = other.height_;
		color_format_ = other.color_format_;
		depth_format_ = other.depth_format_;
		color_type_ = other.color_type_;
		depth_type_ = other.depth_type_;
		color_ = std::move(other.color_);
		depth_ = std::move(other.depth_);

		other.framebuffer_handle_ = NULL;

		return *this;
	}

	template <typename C, typename D>
	framebuffer<C, D>::~framebuffer()
	{
		glDeleteFramebuffers(1, &framebuffer_handle_);
	}

	template <typename C, typename D>
	const C& framebuffer<C, D>::color() const
	{
		return *color_;
	}

	template <typename C, typename D>
	std::shared_ptr<C> framebuffer<C, D>::color_ptr()
	{
		return color_;
	}

	template <typename C, typename D>
	const D& framebuffer<C, D>::depth() const
	{
		return *depth_;
	}

	template <typename C, typename D>
	std::shared_ptr<D> framebuffer<C, D>::depth_ptr()
	{
		return depth_;
	}

	template <typename C, typename D>
	void framebuffer<C, D>::bind(GLenum target) const
	{
		glBindFramebuffer(target, framebuffer_handle_);
	}

	template <typename C, typename D>
	uint framebuffer<C, D>::width() const
	{
		return width_;
	}

	template <typename C, typename D>
	uint framebuffer<C, D>::height() const
	{
		return height_;
	}

	template <typename C, typename D>
	template <typename F>
	void framebuffer<C, D>::blit_to(F& other, GLbitfield buffer)
	{
		GLint draw_bak = 0, read_bak = 0;
		glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read_bak);
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw_bak);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer_handle_);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, other.framebuffer_handle_);

		glBlitFramebuffer(0, 0, width_, height_, 0, 0, other.width_, other.height_, buffer, GL_NEAREST);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, read_bak);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw_bak);
	}

	template <typename C, typename D>
	void framebuffer<C, D>::blit_to_default(GLbitfield buffer) const
	{
		GLint draw_bak = 0, read_bak = 0;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw_bak);
		glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read_bak);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer_handle_);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		// todo: width & height of default
		glBlitFramebuffer(0, 0, width_, height_, 0, 0, width_, height_, buffer, GL_NEAREST);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, read_bak);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw_bak);
	}

	template <typename C, typename D>
	void framebuffer<C, D>::blit_from_default(GLbitfield buffer) const
	{
		GLint draw_bak = 0, read_bak = 0;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw_bak);
		glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read_bak);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer_handle_);

		// todo: width & height of default
		glBlitFramebuffer(0, 0, width_, height_, 0, 0, width_, height_, buffer, GL_NEAREST);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, read_bak);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw_bak);
	}
}
