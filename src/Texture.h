#pragma once


#include "Dodo.h"

namespace dodo
{

	class texture
	{
	private:
		GLuint handle_ = NULL;
		GLenum target_;

	public:

		texture();

		~texture();

		texture(const texture& other) = delete; // todo: implement (how to copy textures?)
		texture(texture&& other) noexcept;
		texture& operator=(const texture& other) = delete; // todo: implement (how to copy textures?)
		texture& operator=(texture&& other) noexcept;

		GLuint handle() const;

		void bind(int uniform_location, int unit = 0) const;


		/**
		 * \brief	Sets a texture parameter. See https://docs.gl/gl4/glTexParameter
		 * \param name 		the name of the parameter
		 * \param value 	the value of the parameter
		 */
		void set_texture_parameter(GLenum name, GLint value) const;

		static texture create_tex_image_2d(GLenum target, GLenum internal_format, GLuint width, GLuint height,
		                                   GLenum format, GLenum type, const GLvoid* data = nullptr);


		static texture from_image_file(const char* path, bool flip_vertical = false);

		static texture cubemap_from_image_files(const char* path_pos_x, const char* path_pos_y,
			const char* path_pos_z, const char* path_neg_x,
			const char* path_neg_y, const char* path_neg_z);

	};


}
