#include "Texture.h"


dodo::texture::texture()
{
	glGenTextures(1, &handle_);
}

dodo::texture::~texture()
{
	glDeleteTextures(1, &handle_);
}

dodo::texture::texture(texture&& other) noexcept
	: handle_(other.handle_), target_(other.target_)
{
	other.handle_ = NULL;
}

dodo::texture& dodo::texture::operator=(texture&& other) noexcept
{
	if (this == &other)
		return *this;
	handle_ = other.handle_;
	target_ = other.target_;
	other.handle_ = NULL;
	return *this;
}

GLuint dodo::texture::handle() const
{
	return handle_;
}

void dodo::texture::bind(int uniform_location, int unit) const
{
	glUniform1i(uniform_location, unit);
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(target_, handle_);
}

void dodo::texture::set_texture_parameter(GLenum name, GLint value) const
{
	glBindTexture(target_, handle_);
	glTexParameteri(target_, name, value);
}


dodo::texture dodo::texture::create_tex_image_2d(GLenum target, GLenum internal_format,
                                               GLuint width, GLuint height, GLenum format,
                                               GLenum type, const GLvoid* data)
{
	texture texture;
	texture.target_ = target;
	glBindTexture(target, texture.handle_);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glTexImage2D(target, 0, internal_format, width, height, 0, format, type, data);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(target, NULL);
	return texture;
}


dodo::texture dodo::texture::from_image_file(const char* path, bool flip_vertical)
{
	texture texture;
	texture.target_ = GL_TEXTURE_2D;

	// bind texture
	glBindTexture(texture.target_, texture.handle_);

	// upload image data to gpu
	int height, width, num_col_ch;
	stbi_set_flip_vertically_on_load_thread(flip_vertical);
	auto* data = stbi_load(path, &width, &height, &num_col_ch, 0);

	if (stbi_failure_reason())
	{
		printf("%s\n", stbi_failure_reason());
		return texture;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	if (num_col_ch == 1)
		glTexImage2D(texture.target_, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
	else if (num_col_ch == 3)
		glTexImage2D(texture.target_, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	else
		glTexImage2D(texture.target_, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);


	// set parameters
	glTexParameteri(texture.target_, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(texture.target_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(texture.target_, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(texture.target_, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexParameteri(texture.target_, GL_TEXTURE_WRAP_T, GL_REPEAT);
	float aniso = 0.0f;
#ifdef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
	glTexParameterf(texture.target_, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
#elif defined(GL_MAX_TEXTURE_MAX_ANISOTROPY)
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &aniso);
	glTexParameterf(texture.target_, GL_TEXTURE_MAX_ANISOTROPY, aniso);
#endif

	// generate mipmaps
	glGenerateMipmap(texture.target_);

	stbi_image_free(data);

	glBindTexture(texture.target_, NULL);

	return texture;
}


dodo::texture dodo::texture::cubemap_from_image_files(const char* path_pos_x, const char* path_pos_y,
                                                  const char* path_pos_z,
                                                  const char* path_neg_x, const char* path_neg_y,
                                                  const char* path_neg_z)
{
	dodo::texture cube_map;
	cube_map.target_ = GL_TEXTURE_CUBE_MAP;

	glBindTexture(cube_map.target_, cube_map.handle_);

	const char* path[6] = {
		path_pos_x, path_pos_y, path_pos_z,
		path_neg_x, path_neg_y, path_neg_z
	};
	constexpr int targets[6] = {
		GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	};

	stbi_set_flip_vertically_on_load_thread(false);

	for (int i = 0; i < 6; i++)
	{
		static int height, width, num_col_ch;
		auto* data = stbi_load(path[i], &height, &width, &num_col_ch, 0);

		// todo: check if loading failed

		if (num_col_ch == 3)
			glTexImage2D(targets[i], 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		else
			glTexImage2D(targets[i], 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);
	}

	glTexParameteri(cube_map.target_, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(cube_map.target_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(cube_map.target_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(cube_map.target_, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(cube_map.target_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenerateMipmap(cube_map.target_);

	glBindTexture(cube_map.target_, NULL);

	return cube_map;
}
