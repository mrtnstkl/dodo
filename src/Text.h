#pragma once

#include <bitset>
#include <list>
#include <memory>
#include <sstream>
#include <string>
#include "Framebuffer.h"
#include "Instanced.h"
#include "Material.h"
#include "Texture.h"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"


namespace dodo
{
	class font
	{
	public:
		struct glyph
		{
			static constexpr size_t width = 8;
			static constexpr size_t height = 12;
			using bitset = std::bitset<glyph::width * glyph::height>;
			int atlas_id;
			bitset pixels;
			std::string to_string() const;
		};

	private:
		std::shared_ptr<texture> font_atlas_;
		glm::vec2 font_atlas_dimensions_;
		std::unordered_map<char32_t, glyph> glyphs_;
		size_t glyph_count_;
		size_t glyphs_per_row_;

		static std::unordered_map<char32_t, glyph::bitset> load_embedded_font();
		void initialize_font(const std::unordered_map<char32_t, dodo::font::glyph::bitset> &glyph_bitsets);
		font(); // private constructor for default font. get it via font::default_font()
	public:
		font(const char *path); // todo: implement loading from file
		int char_to_atlas_id(char32_t ch) const;
		glyph char_to_glyph(char32_t ch) const;
		const texture &atlas();
		size_t glyph_count() const;
		size_t glyphs_per_row() const;
		glm::vec2 glyph_uv_scale() const;
		static std::shared_ptr<font> default_font();
	};

	class text_block
	{
	public:
		friend class renderer;
		static const char *shader_source_vertex;
		static const char *shader_source_fragment;
		struct billboard_vertex
		{
			float x, y;
			static void configure_vao(GLuint vao, GLuint vbo, GLuint vib);
			static constexpr GLuint first_free_input_location = 1;
		} __attribute__((packed, aligned(1)));
		struct character_instance : instanced::array_structs::instanced_array_struct
		{
			int atlas_id;
			static void bind(GLuint starting_index);
		} __attribute__((packed, aligned(1)));


		size_t columns_, rows_;
		std::list<std::u32string> lines_;
		instanced::instanced_mesh<billboard_vertex, character_instance> mesh_;
		std::shared_ptr<font> font_;

		framebuffer::framebuffer<texture> canvas_; // todo: unused
		std::stringstream input_;
		bool needs_redraw_ = true;

		glm::vec3 color_ = dodo::color::WHITE;
		bool wrap_;
		bool follow_;

		void update();

	public:
		/**
		 * \brief Constructs a text block with the given number of rows and columns.
		 *
		 * \param rows		the number of rows in the text block
		 * \param columns	the number of columns in the text block
		 * \param wrap		whether to wrap text when it exceeds the number of columns
		 * \param follow	whether to scroll to the end when new lines are added
		 */
		text_block(int rows, int columns, bool wrap = true, bool follow = true);

		// TODO: define copy & move behaviour

		/**
		 * \brief Clears all text from the textblock.
		 */
		void clear();

		/**
		 * \brief Appends to the end of the text block.
		 *
		 * \param value	the value to append; can be any type that can be streamed into a std::stringstream
		 */
		template <typename T>
		text_block &operator<<(T value);

		/**
		 * \brief Returns the stringstream for text insertion.
		 */
		std::stringstream &input();
	};
} // namespace dodo


template <typename T>
dodo::text_block &dodo::text_block::operator<<(T value)
{
	input_ << value;
	return *this;
}
