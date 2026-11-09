#pragma once


#include <memory>
#include "Dodo.h"
#include "Camera.h"
#include "Light.h"
#include "Material.h"
#include "Model.h"
#include "Shader.h"
#include "Text.h"
#include "glm/ext/vector_float2.hpp"


namespace dodo
{


	class renderer
	{
	public:
		using render_flag_mask = uint32_t;
		struct render_flags
		{
			static constexpr render_flag_mask none = 0;
			static constexpr render_flag_mask depth_test = 1 << 0;
			static constexpr render_flag_mask backface_culling = 1 << 1;
			static constexpr render_flag_mask blend = 1 << 2;
			static constexpr render_flag_mask wireframe = 1 << 3;
			static constexpr render_flag_mask points = 1 << 4;
			static constexpr render_flag_mask nearest_sampling = 1 << 5;
			static constexpr render_flag_mask no_depth_write = 1 << 6;
			static constexpr render_flag_mask custom = 1 << 7;
		};

	private:
		glm::vec4 clear_color_ = { 1, 1, 1, 1 };

		std::list<std::weak_ptr<directional_light>> directional_light_;
		std::list<std::weak_ptr<point_light>> point_light_;
		std::weak_ptr<dodo::camera> camera_;

		render_flag_mask general_flags_ = render_flags::depth_test | render_flags::backface_culling;
		render_flag_mask temp_positive_flags_ = 0;
		render_flag_mask temp_negative_flags_ = 0;

		GLuint nearest_texture_sampler;

		const struct
		{
			const standard_mesh skybox_mesh;
			const standard_mesh render_quad;
		} utilities_;
		struct
		{
			std::unique_ptr<shader::shader_program> text;
		} shaders_;

		virtual void apply_render_flags(render_flag_mask flag_mask);

	public:
		void apply_render_flags();

		renderer();
		virtual ~renderer();

		/**
		 * \brief	Clears the back-buffer.
		 * \param mask	mask of buffers to be cleared
		 */
		void clear(GLbitfield mask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) const;

		/**
		 * \brief	Renders a 2x2 rect (-1 to 1 x&y). Like render_texture, but without the texture.
		 * \param shader	the shader to be used for rendering
		 */
		void render(const shader::shader_program& shader);

		/**
		 * \brief	Renders a 3d-model.
		 * \param model			the model to be rendered
		 * \param shader		the shader to be used for rendering
		 * \param manual_mode	when enabled, the renderer will not change opengl's state,
		 *						except for binding the vertex array and shader (glBindVertexArray, glUseProgram)
		 */
		void render_model(const model& model, const shader::shader_program& shader, bool manual_mode = false, GLenum index_layout = GL_TRIANGLES);

		/**
		 * \brief	Renders a cubemap as a skybox.
		 * \param cube_map	the cubemap of the skybox
		 * \param shader	the shader to be used for rendering
		 */
		void render_skybox(const texture& cube_map, const shader::shader_program& shader);

		/**
		 * \brief	Renders a texture to a 2x2 rect (-1 to 1 x&y). Can be useful for post-processing.
		 * \param texture	the texture to be rendered
		 * \param shader	the shader to be used for rendering
		 */
		void render_texture(const texture& texture, const shader::shader_program& shader);

		/**
		 * \brief	Renders a text block.
		 * \param text	the text block to be rendered
		 * \param pos	the position of the text block. top left is (0,0), bottom right is (1,1)
		 * \param scale	the scaling of the text block
		 */
		void render_text(text_block& text, glm::vec2 pos = {0, 0}, float scale = 2.f, glm::vec3 color = color::WHITE);

		/**
		 * \brief	Configures opengl's state based on the flags that have been set for the renderer,
		 *			as well as the flags passed in the arguments.
		 * \note	The render functions do this automatically, so probably don't use it in combination with them.
		 */
		void prepare_state(render_flag_mask positive_flags, render_flag_mask negative_flags = 0);

		/**
		 * \brief	Adds a directional light to the group of light sources.
		 * \param directional_light		the directional light
		 */
		void add_directional_light(const std::weak_ptr<directional_light>& directional_light);

		/**
		 * \brief	Adds a point light to the group of light sources.
		 * \param point_light 	the point light
		 */
		void add_point_light(const std::weak_ptr<point_light>& point_light);


		/**
		 * \brief	Sets the clear color used for clearing the color buffer.
		 * \param clear_color	the new clear color
		 */
		void set_clear_color(const glm::vec4& clear_color);

		void set_clear_color(const glm::vec3& clear_color);

		void set_camera(const std::shared_ptr<dodo::camera>& camera);

		[[deprecated]] void set_backface_culling(bool b);

		[[deprecated]] void set_depth_test(bool b);

		[[deprecated]] void set_wireframe(bool b);

		[[deprecated]] void set_point_rendering(bool b);

		[[deprecated]] void set_blend(bool b);

		/**
		 * \brief	Sets render flags.
		 * \param flags	the new flag values
		 * \param mask	a mask that limits the flags to be modified
		 */
		void set_flags(render_flag_mask flags, render_flag_mask mask = UINT32_MAX);

		/**
		 * \brief	Enables certain render flags.
		 * \param positive_flags	the flags to enable. see dodo::renderer::render_flags::
		 */
		void enable_flags(render_flag_mask positive_flags);

		/**
		 * \brief	Disables certain render flags.
		 * \param negative_flags	the flags to disable. see dodo::renderer::render_flags::
		 */
		void disable_flags(render_flag_mask negative_flags);

		/**
		 * \brief	Returns a bitmask of the currently active render flags.
		 * \note	temporary flags set by next_call() are ignored
		 */
		render_flag_mask flags() const;

		/**
		 * \brief	Enables and disables certain flags for the next render call
		 *			(render, render_model, render_skybox, render_texture).
		 * \param positive_flags	flags that will be enabled for the next call
		 * \param negative_flags	flags that will be disabled for the next call
		 */
		void next_call(render_flag_mask positive_flags, render_flag_mask negative_flags = 0);

		// was renamed to 'next_call'
		[[deprecated]] void set_temporary_flags(render_flag_mask positive_flags, render_flag_mask negative_flags = 0) = delete;

		[[nodiscard]] glm::vec4 clear_color() const;

		[[nodiscard]] std::weak_ptr<dodo::camera> camera() const;

		[[nodiscard]] [[deprecated]] bool backface_culling() const;

		[[nodiscard]] [[deprecated]] bool depth_test() const;

		[[nodiscard]] [[deprecated]] bool wireframe() const;
	};
}
