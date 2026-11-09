#include "Text.h"
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include "Texture.h"

namespace
{
	// forward declaration of embedded font
	extern const char *monogram_font_encoded;
} // namespace

static std::u32string decode_utf8(std::string_view text);

std::string dodo::font::glyph::to_string() const
{
	std::string result;
	for (size_t y = 0; y < glyph::height; ++y)
	{
		for (size_t x = 0; x < glyph::width; ++x)
			result += pixels[y * glyph::width + x] ? '#' : ':';
		result += '\n';
	}
	return result;
}

std::unordered_map<char32_t, dodo::font::glyph::bitset> dodo::font::load_embedded_font()
{
	std::stringstream encoded_font;
	encoded_font << monogram_font_encoded;

	std::unordered_map<char32_t, dodo::font::glyph::bitset> font;
	std::string line;
	size_t line_number = 0;

	while (std::getline(encoded_font, line))
	{
		++line_number;
		const auto start = line.find_first_not_of(" \t\r");
		if (start == std::string::npos || line[start] == '#')
			continue;
		std::istringstream stream(line);
		stream >> std::hex;

		std::uint32_t code_point;
		if (!(stream >> code_point) || code_point > 0x10FFFF || (code_point >= 0xD800 && code_point <= 0xDFFF))
			throw std::runtime_error("Invalid code point on line " + std::to_string(line_number));

		dodo::font::glyph::bitset glyph;
		for (std::size_t y = 0; y < dodo::font::glyph::height; ++y)
		{
			unsigned int row;
			if (!(stream >> row) || row > 0xFF)
				throw std::runtime_error("Invalid bitmap row on line " + std::to_string(line_number));
			for (std::size_t x = 0; x < dodo::font::glyph::width; ++x)
			{
				if ((row >> x) & 1u)
					glyph.set(y * dodo::font::glyph::width + x);
			}
		}

		std::string extra;
		if (stream >> extra)
			throw std::runtime_error("Extra fields on line " + std::to_string(line_number));
		if (!font.emplace(static_cast<char32_t>(code_point), glyph).second)
			throw std::runtime_error("Duplicate code point on line " + std::to_string(line_number));
	}
	return font;
}

void dodo::font::initialize_font(const std::unordered_map<char32_t, dodo::font::glyph::bitset> &glyph_bitsets)
{
	glyph_count_ = glyph_bitsets.size() + 1; // +1 for the missing/empty glyph
	glyphs_per_row_ = sqrt(glyph_count_) + 1;
	const size_t atlas_width = glyphs_per_row_ * glyph::width;
	const size_t atlas_height = glyphs_per_row_ * glyph::height;
	font_atlas_dimensions_ = {static_cast<float>(atlas_width), static_cast<float>(atlas_height)};

	uint8_t *data = new uint8_t[atlas_width * atlas_height];

	glyphs_.emplace(0, glyph{0, glyph::bitset()}); // missing glyph
	for (size_t i = 0; i < glyph::height; ++i)
		for (size_t j = 0; j < glyph::width; ++j)
			data[i * atlas_width + j] = 0;
	int atlas_id = 1;
	for (const auto &[ch, bitset] : glyph_bitsets)
	{
		const size_t top = (atlas_id / glyphs_per_row_) * glyph::height;
		const size_t left = (atlas_id % glyphs_per_row_) * glyph::width;
		for (size_t y = 0; y < glyph::height; ++y)
			for (size_t x = 0; x < glyph::width; ++x)
				data[(top + y) * atlas_width + (left + x)] = bitset[y * glyph::width + x] ? 255 : 0;
		glyphs_.emplace(ch, glyph{atlas_id, bitset});
		++atlas_id;
	}

	font_atlas_ = std::make_shared<dodo::texture>(
		texture::create_tex_image_2d(GL_TEXTURE_2D, GL_RED, atlas_width, atlas_height, GL_RED, GL_UNSIGNED_BYTE, data)
	);
	delete[] data;
}

dodo::font::font() { initialize_font(load_embedded_font()); }

dodo::font::glyph dodo::font::char_to_glyph(char32_t ch) const
{
	if (auto it = glyphs_.find(ch); it != glyphs_.end())
		return it->second;
	else
		return glyphs_.at(U'?'); // todo: dont hardcode this here
}

int dodo::font::char_to_atlas_id(char32_t ch) const
{
	if (auto it = glyphs_.find(ch); it != glyphs_.end())
		return it->second.atlas_id;
	else
		return glyphs_.at(U'?').atlas_id; // todo: dont hardcode this here
}

const dodo::texture &dodo::font::atlas() { return *font_atlas_; }

size_t dodo::font::glyph_count() const { return glyph_count_; }

size_t dodo::font::glyphs_per_row() const { return glyphs_per_row_; }

glm::vec2 dodo::font::glyph_uv_scale() const
{
	return {static_cast<float>(glyph::width) / font_atlas_dimensions_.x,
			static_cast<float>(glyph::height) / font_atlas_dimensions_.y};
}

std::shared_ptr<dodo::font> dodo::font::default_font()
{
	static auto default_font_instance = std::shared_ptr<dodo::font>(new dodo::font());
	return default_font_instance;
}

void dodo::text_block::billboard_vertex::configure_vao(GLuint vao, GLuint vbo, GLuint vib)
{
	// bind VAO, VBO
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// enable attributes
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(billboard_vertex), (void *)0);
	// bind index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vib);
	// unbind VAO, VBO
	glBindVertexArray(NULL);
	glBindBuffer(GL_ARRAY_BUFFER, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);
}

void dodo::text_block::character_instance::bind(GLuint starting_index)
{
	glEnableVertexAttribArray(starting_index);
	glVertexAttribIPointer(starting_index, 1, GL_INT, sizeof(int), (void *)0);
	glVertexAttribDivisor(starting_index, 1);
}

void dodo::text_block::update()
{
	auto stream_view = input_.view();
	if (!stream_view.empty())
	{
		// process new input
		const auto u32text = decode_utf8(stream_view);
		// std::cout << "Decoded UTF-8 text: " << std::string(u32text.begin(), u32text.end()) << std::endl;
		for (const auto ch : u32text)
		{
			switch (ch)
			{
			case U'\n':
				lines_.emplace_back();
				break;
			case U'\r': // discard carriage return
				break;
			default:
				if (lines_.back().size() < columns_)
					lines_.back() += ch;
				else if (wrap_)
					lines_.emplace_back() += ch;
			}
			if (lines_.size() > rows_)
				lines_.erase(follow_ ? lines_.begin() : std::prev(lines_.end()));
		}
		needs_redraw_ = true;
		input_.str(std::string());
		input_.clear();
	}
	if (!needs_redraw_)
		return;

	const auto set = [&, w = columns_](size_t row, size_t col, int atlas_id)
	{ mesh_[row * w + col].atlas_id = atlas_id; };

	auto line = lines_.begin();
	for (size_t current_row = 0; current_row < rows_; ++current_row)
	{
		size_t current_col = 0;
		if (line != lines_.end())
		{
			for (auto ch : *line)
				set(current_row, current_col++, font_->char_to_atlas_id(ch));
			++line;
		}
		while (current_col < columns_)
			set(current_row, current_col++, 0);
	}
	mesh_.update_instance_buffer();
}

dodo::text_block::text_block(int rows, int columns, bool wrap, bool follow)
	: columns_(columns)
	, rows_(rows)
	, lines_(1)
	, mesh_(columns * rows)
	, font_(font::default_font())
	, wrap_(wrap)
	, follow_(follow)
{
	const auto v1 = mesh_.add_vertex({0, 0});
	const auto v2 = mesh_.add_vertex({1, 0});
	const auto v3 = mesh_.add_vertex({0, 1});
	const auto v4 = mesh_.add_vertex({1, 1});
	mesh_.add_triangle(v1, v3, v2);
	mesh_.add_triangle(v2, v3, v4);

	mesh_.write_buffers();
	mesh_.update_instance_buffer();
	clear();
}

// const dodo::texture &dodo::text_block::texture() const { return canvas_.color(); }

void dodo::text_block::clear()
{
	lines_.clear();
	lines_.emplace_back();
	needs_redraw_ = true;
}

size_t dodo::text_block::width() const { return columns_ * font::glyph::width; }

size_t dodo::text_block::height() const { return rows_ * font::glyph::height; }

size_t dodo::text_block::rows() const { return rows_; }

size_t dodo::text_block::columns() const { return columns_; }

std::stringstream &dodo::text_block::input() { return input_; }

static std::u32string decode_utf8(std::string_view text)
{
	std::u32string result;

	for (size_t i = 0; i < text.size();)
	{
		unsigned char c = text[i++];
		char32_t cp;
		int extra;

		if (c < 0x80)
		{
			cp = c;
			extra = 0;
		}
		else if ((c & 0xE0) == 0xC0)
		{
			cp = c & 0x1F;
			extra = 1;
		}
		else if ((c & 0xF0) == 0xE0)
		{
			cp = c & 0x0F;
			extra = 2;
		}
		else if ((c & 0xF8) == 0xF0)
		{
			cp = c & 0x07;
			extra = 3;
		}
		else
		{
			throw std::runtime_error("Invalid UTF-8");
		}

		if (i + extra > text.size())
			throw std::runtime_error("Truncated UTF-8");

		for (int j = 0; j < extra; ++j)
		{
			unsigned char next = text[i++];
			if ((next & 0xC0) != 0x80)
				throw std::runtime_error("Invalid UTF-8");
			cp = (cp << 6) | (next & 0x3F);
		}

		if ((extra == 1 && cp < 0x80) || (extra == 2 && cp < 0x800) || (extra == 3 && cp < 0x10000) ||
			(cp >= 0xD800 && cp <= 0xDFFF) || cp > 0x10FFFF)
			throw std::runtime_error("Invalid UTF-8");

		result.push_back(cp);
	}
	return result;
}


const char *dodo::text_block::shader_source_vertex = R"(#version 430 core
layout(location = 0) in vec2 position; // vertex position of the quad (0,0) to (1,1)
layout(location = 1) in int atlas_id; // the atlas ID of the glyph to render
uniform int atlas_glyphs_per_row; // number of glyphs per row in the atlas
uniform vec2 atlas_glyph_scale; // width and height of a glyph in the atlas
uniform ivec2 grid_size; // number of glyphs per row and column in the text block
uniform vec2 cell_size; // width and height of a glyph
uniform float scale;
uniform vec2 origin;
out vec2 uv;

void main() {
	vec2 uv_offset = vec2(atlas_id % atlas_glyphs_per_row, atlas_id / atlas_glyphs_per_row) * atlas_glyph_scale;
	uv = position * atlas_glyph_scale + uv_offset;
	vec2 instance_pos = (vec2(
		gl_InstanceID % grid_size.x,
		gl_InstanceID / grid_size.x
	) + position) * cell_size * scale;
	gl_Position = vec4(instance_pos.x - 1 + origin.x * 2, 1 - instance_pos.y - origin.y * 2, 0, 1);
}
)";

const char *dodo::text_block::shader_source_fragment = R"(#version 430 core
in vec2 uv;
uniform sampler2D atlas;
uniform vec3 color;
out vec4 fragColor;

void main() {
	float value = texture(atlas, uv).r;
	fragColor = vec4(color, value);
	//fragColor = texture(atlas, uv);
}
)";

namespace
{
	// Monogram bitmap font: 8 columns x 12 rows, one glyph per line.
	// Format: Unicode_codepoint_in_hex followed by 12 hex row bytes (top to bottom).
	// Within each row, bit 0 is the LEFTMOST pixel; bit 7 is the rightmost.
	// Example: 0E = .###.... (left to right); 11 = #...#...
	// Note: glyphs normally occupy 5 columns; a few use bits 5/6.
	const char *monogram_font_encoded =
		R"(0030 00 00 00 0E 11 19 15 13 11 0E 00 00
0031 00 00 00 04 06 04 04 04 04 1F 00 00
0032 00 00 00 0E 11 10 08 04 02 1F 00 00
0033 00 00 00 0E 11 10 0C 10 11 0E 00 00
0034 00 00 00 12 12 11 1F 10 10 10 00 00
0035 00 00 00 1F 01 0F 10 10 11 0E 00 00
0036 00 00 00 0E 01 01 0F 11 11 0E 00 00
0037 00 00 00 1F 10 10 08 04 04 04 00 00
0038 00 00 00 0E 11 11 0E 11 11 0E 00 00
0039 00 00 00 0E 11 11 1E 10 11 0E 00 00
0021 00 00 00 04 04 04 04 04 00 04 00 00
0022 00 00 00 0A 0A 0A 00 00 00 00 00 00
0023 00 00 00 00 0A 1F 0A 0A 1F 0A 00 00
0024 00 00 00 04 1E 05 0E 14 0F 04 00 00
0025 00 00 00 11 11 08 04 02 11 11 00 00
0026 00 00 00 06 09 09 1E 09 09 16 00 00
0027 00 00 00 04 04 04 00 00 00 00 00 00
0028 00 00 00 08 04 04 04 04 04 08 00 00
0029 00 00 00 02 04 04 04 04 04 02 00 00
002A 00 00 00 00 04 15 0E 15 04 00 00 00
002B 00 00 00 00 04 04 1F 04 04 00 00 00
002C 00 00 00 00 00 00 00 00 04 04 02 00
002D 00 00 00 00 00 00 1F 00 00 00 00 00
002E 00 00 00 00 00 00 00 00 04 04 00 00
002F 00 00 00 10 10 08 04 02 01 01 00 00
003A 00 00 00 00 04 04 00 00 04 04 00 00
003B 00 00 00 00 04 04 00 00 04 04 02 00
003C 00 00 00 00 18 06 01 06 18 00 00 00
003D 00 00 00 00 00 1F 00 1F 00 00 00 00
003E 00 00 00 00 03 0C 10 0C 03 00 00 00
003F 00 00 00 0E 11 10 08 04 00 04 00 00
0040 00 00 00 0E 19 15 15 19 01 0E 00 00
0041 00 00 00 0E 11 11 11 1F 11 11 00 00
0042 00 00 00 0F 11 11 0F 11 11 0F 00 00
0043 00 00 00 0E 11 01 01 01 11 0E 00 00
0044 00 00 00 0F 11 11 11 11 11 0F 00 00
0045 00 00 00 1F 01 01 0F 01 01 1F 00 00
0046 00 00 00 1F 01 01 0F 01 01 01 00 00
0047 00 00 00 0E 11 01 1D 11 11 0E 00 00
0048 00 00 00 11 11 11 1F 11 11 11 00 00
0049 00 00 00 1F 04 04 04 04 04 1F 00 00
004A 00 00 00 10 10 10 10 11 11 0E 00 00
004B 00 00 00 11 09 05 03 05 09 11 00 00
004C 00 00 00 01 01 01 01 01 01 1F 00 00
004D 00 00 00 11 1B 15 11 11 11 11 00 00
004E 00 00 00 11 11 13 15 19 11 11 00 00
004F 00 00 00 0E 11 11 11 11 11 0E 00 00
0050 00 00 00 0F 11 11 0F 01 01 01 00 00
0051 00 00 00 0E 11 11 11 11 11 0E 18 00
0052 00 00 00 0F 11 11 0F 11 11 11 00 00
0053 00 00 00 0E 11 01 0E 10 11 0E 00 00
0054 00 00 00 1F 04 04 04 04 04 04 00 00
0055 00 00 00 11 11 11 11 11 11 0E 00 00
0056 00 00 00 11 11 11 11 0A 0A 04 00 00
0057 00 00 00 11 11 11 11 15 1B 11 00 00
0058 00 00 00 11 11 0A 04 0A 11 11 00 00
0059 00 00 00 11 11 0A 04 04 04 04 00 00
005A 00 00 00 1F 10 08 04 02 01 1F 00 00
005B 00 00 00 0C 04 04 04 04 04 0C 00 00
005C 00 00 00 01 01 02 04 08 10 10 00 00
005D 00 00 00 06 04 04 04 04 04 06 00 00
005E 00 00 00 04 0A 11 00 00 00 00 00 00
005F 00 00 00 00 00 00 00 00 00 1F 00 00
0060 00 00 00 02 04 00 00 00 00 00 00 00
0061 00 00 00 00 00 1E 11 11 11 1E 00 00
0062 00 00 00 01 01 0F 11 11 11 0F 00 00
0063 00 00 00 00 00 0E 11 01 11 0E 00 00
0064 00 00 00 10 10 1E 11 11 11 1E 00 00
0065 00 00 00 00 00 0E 11 1F 01 0E 00 00
0066 00 00 00 0C 12 02 0F 02 02 02 00 00
0067 00 00 00 00 00 1E 11 11 11 1E 10 0E
0068 00 00 00 01 01 0F 11 11 11 11 00 00
0069 00 00 00 04 00 06 04 04 04 1F 00 00
006A 00 00 00 10 00 18 10 10 10 10 11 0E
006B 00 00 00 01 01 11 09 07 09 11 00 00
006C 00 00 00 03 02 02 02 02 02 1C 00 00
006D 00 00 00 00 00 0F 15 15 15 15 00 00
006E 00 00 00 00 00 0F 11 11 11 11 00 00
006F 00 00 00 00 00 0E 11 11 11 0E 00 00
0070 00 00 00 00 00 0F 11 11 11 0F 01 01
0071 00 00 00 00 00 1E 11 11 11 1E 10 10
0072 00 00 00 00 00 0D 13 01 01 01 00 00
0073 00 00 00 00 00 1E 01 0E 10 0F 00 00
0074 00 00 00 02 02 0F 02 02 02 1C 00 00
0075 00 00 00 00 00 11 11 11 11 1E 00 00
0076 00 00 00 00 00 11 11 11 0A 04 00 00
0077 00 00 00 00 00 11 11 15 15 0A 00 00
0078 00 00 00 00 00 11 0A 04 0A 11 00 00
0079 00 00 00 00 00 11 11 11 11 1E 10 0E
007A 00 00 00 00 00 1F 08 04 02 1F 00 00
007B 00 00 00 08 04 04 02 04 04 08 00 00
007C 00 00 00 04 04 04 04 04 04 04 00 00
007D 00 00 00 02 04 04 08 04 04 02 00 00
007E 00 00 00 00 00 12 0D 00 00 00 00 00
00A1 00 00 00 04 00 04 04 04 04 04 00 00
00A2 00 00 00 04 0E 15 05 15 0E 04 00 00
00A3 00 00 00 0C 12 02 0F 02 02 1F 00 00
00A4 00 00 00 00 11 0E 0A 0E 11 00 00 00
00A5 00 00 00 11 0A 04 1F 04 1F 04 00 00
00A6 00 00 00 04 04 04 00 04 04 04 00 00
00A7 00 00 00 1E 01 0E 11 0E 10 0F 00 00
00A8 00 00 00 0A 00 00 00 00 00 00 00 00
00A9 00 00 00 0E 1B 15 1D 15 1B 0E 00 00
00AA 00 00 00 0E 09 09 09 0E 00 00 00 00
00AB 00 00 00 00 00 12 09 12 00 00 00 00
00AC 00 00 00 00 00 00 1F 10 00 00 00 00
00AE 00 00 00 0E 19 15 15 19 15 0E 00 00
00AF 00 00 00 00 00 00 1F 00 00 00 00 00
00B0 00 00 00 06 09 09 06 00 00 00 00 00
00B1 00 00 00 04 04 1F 04 04 00 1F 00 00
00B2 00 00 00 03 04 02 01 07 00 00 00 00
00B3 00 00 00 03 04 02 04 03 00 00 00 00
00B4 00 00 08 04 00 00 00 00 00 00 00 00
00B5 00 00 00 00 00 11 11 11 11 0F 01 01
00B6 00 00 00 1E 17 17 17 16 14 14 00 00
00B7 00 00 00 04 00 00 00 00 00 00 00 00
00B8 00 00 00 00 00 00 00 00 00 04 08 06
00B9 00 00 00 02 03 02 02 07 00 00 00 00
00BA 00 00 00 06 09 09 09 06 00 00 00 00
00BB 00 00 00 00 00 09 12 09 00 00 00 00
00BC 00 00 00 01 09 05 02 15 1C 10 00 00
00BD 00 00 00 01 09 05 0E 11 08 1C 00 00
00BE 00 00 00 07 16 0F 04 16 1D 10 00 00
00BF 00 00 00 04 00 04 02 01 11 0E 00 00
00C0 02 04 00 0E 11 11 1F 11 11 11 00 00
00C1 08 04 00 0E 11 11 1F 11 11 11 00 00
00C2 04 0A 00 0E 11 11 1F 11 11 11 00 00
00C3 16 09 00 0E 11 11 1F 11 11 11 00 00
00C4 00 0A 00 0E 11 11 1F 11 11 11 00 00
00C5 04 0A 04 0E 11 11 1F 11 11 11 00 00
00C6 00 00 00 1E 05 05 1F 05 05 1D 00 00
00C7 00 00 00 0E 11 01 01 01 11 0E 08 06
00C8 02 04 00 1F 01 01 0F 01 01 1F 00 00
00C9 08 04 00 1F 01 01 0F 01 01 1F 00 00
00CA 04 0A 00 1F 01 01 0F 01 01 1F 00 00
00CB 00 0A 00 1F 01 01 0F 01 01 1F 00 00
00CC 02 04 00 1F 04 04 04 04 04 1F 00 00
00CD 08 04 00 1F 04 04 04 04 04 1F 00 00
00CE 04 0A 00 1F 04 04 04 04 04 1F 00 00
00CF 00 0A 00 1F 04 04 04 04 04 1F 00 00
00D0 00 00 00 0F 11 11 13 11 11 0F 00 00
00D1 16 09 00 11 11 13 15 19 11 11 00 00
00D2 02 04 00 0E 11 11 11 11 11 0E 00 00
00D3 08 04 00 0E 11 11 11 11 11 0E 00 00
00D4 04 0A 00 0E 11 11 11 11 11 0E 00 00
00D5 16 09 00 0E 11 11 11 11 11 0E 00 00
00D6 00 0A 00 0E 11 11 11 11 11 0E 00 00
00D7 00 00 00 00 00 00 0A 04 0A 00 00 00
00D8 00 00 00 16 09 19 15 13 12 0D 00 00
00D9 02 04 00 11 11 11 11 11 11 0E 00 00
00DA 08 04 00 11 11 11 11 11 11 0E 00 00
00DB 04 0A 00 11 11 11 11 11 11 0E 00 00
00DC 00 0A 00 11 11 11 11 11 11 0E 00 00
00DD 08 04 00 11 11 0A 04 04 04 04 00 00
00DE 00 00 00 01 0F 11 11 11 0F 01 00 00
00DF 00 00 00 06 09 09 0D 11 11 0D 00 00
00E0 00 00 02 04 00 1E 11 11 11 1E 00 00
00E1 00 00 08 04 00 1E 11 11 11 1E 00 00
00E2 00 00 04 0A 00 1E 11 11 11 1E 00 00
00E3 00 00 16 09 00 1E 11 11 11 1E 00 00
00E4 00 00 00 0A 00 1E 11 11 11 1E 00 00
00E5 00 04 0A 04 00 1E 11 11 11 1E 00 00
00E6 00 00 00 00 00 0E 15 1D 05 1E 00 00
00E7 00 00 00 00 00 0E 11 01 11 0E 08 06
00E8 00 00 02 04 00 0E 11 1F 01 0E 00 00
00E9 00 00 08 04 00 0E 11 1F 01 0E 00 00
00EA 00 00 04 0A 00 0E 11 1F 01 0E 00 00
00EB 00 00 00 0A 00 0E 11 1F 01 0E 00 00
00EC 00 00 02 04 00 06 04 04 04 1F 00 00
00ED 00 00 08 04 00 06 04 04 04 1F 00 00
00EE 00 00 04 0A 00 06 04 04 04 1F 00 00
00EF 00 00 00 0A 00 06 04 04 04 1F 00 00
00F0 00 00 0E 30 18 1E 11 11 11 0E 00 00
00F1 00 00 16 09 00 0F 11 11 11 11 00 00
00F2 00 00 02 04 00 0E 11 11 11 0E 00 00
00F3 00 00 08 04 00 0E 11 11 11 0E 00 00
00F4 00 00 04 0A 00 0E 11 11 11 0E 00 00
00F5 00 00 16 09 00 0E 11 11 11 0E 00 00
00F6 00 00 00 0A 00 0E 11 11 11 0E 00 00
00F7 00 00 00 00 00 04 00 1F 00 04 00 00
00F8 00 00 00 00 00 16 09 15 12 0D 00 00
00F9 00 00 02 04 00 11 11 11 11 1E 00 00
00FA 00 00 08 04 00 11 11 11 11 1E 00 00
00FB 00 00 04 0A 00 11 11 11 11 1E 00 00
00FC 00 00 00 0A 00 11 11 11 11 1E 00 00
00FD 00 00 08 04 00 11 11 11 11 1E 10 0E
00FE 00 00 00 01 01 0F 11 11 11 0F 01 01
00FF 00 00 00 0A 00 11 11 11 11 1E 10 0E
0100 00 0E 00 0E 11 11 1F 11 11 11 00 00
0101 00 00 00 0E 00 1E 11 11 11 1E 00 00
0102 0A 04 00 0E 11 11 1F 11 11 11 00 00
0103 00 00 0A 04 00 1E 11 11 11 1E 00 00
0104 00 00 00 0E 11 11 1F 11 11 11 08 10
0105 00 00 00 00 00 1E 11 11 11 1E 04 18
0106 08 04 00 0E 11 01 01 01 11 0E 00 00
0107 00 00 08 04 00 0E 11 01 11 0E 00 00
0108 04 0A 00 0E 11 01 01 01 11 0E 00 00
0109 00 00 04 0A 00 0E 11 01 11 0E 00 00
010A 00 04 00 0E 11 01 01 01 11 0E 00 00
010B 00 00 00 04 00 0E 11 01 11 0E 00 00
010C 0A 04 00 0E 11 01 01 01 11 0E 00 00
010D 00 00 0A 04 00 0E 11 01 11 0E 00 00
010E 0A 04 00 0F 11 11 11 11 11 0F 00 00
010F 00 00 50 50 10 1E 11 11 11 1E 00 00
0110 00 00 00 0F 11 11 13 11 11 0F 00 00
0111 00 00 10 3C 10 1E 11 11 11 1E 00 00
0112 00 0E 00 1F 01 01 07 01 01 1F 00 00
0113 00 00 00 0E 00 0E 11 1F 01 0E 00 00
0114 0A 04 00 1F 01 01 07 01 01 1F 00 00
0115 00 00 0A 04 00 0E 11 1F 01 0E 00 00
0116 00 04 00 1F 01 01 07 01 01 1F 00 00
0117 00 00 00 04 00 0E 11 1F 01 0E 00 00
0118 00 00 00 1F 01 01 07 01 01 1F 04 18
0119 00 00 00 00 00 0E 11 1F 01 1E 04 18
011A 00 0E 00 1F 01 01 07 01 01 1F 00 00
011B 00 00 00 0A 00 0E 11 1F 01 0E 00 00
011C 04 0A 00 0E 11 01 1D 11 11 0E 00 00
011D 00 00 04 0A 00 1E 11 11 11 1E 10 0E
011E 0A 04 00 0E 11 01 1D 11 11 0E 00 00
011F 00 00 0A 04 00 1E 11 11 11 1E 10 0E
0120 00 04 00 0E 11 01 1D 11 11 0E 00 00
0121 00 00 00 04 00 1E 11 11 11 1E 10 0E
0122 00 00 00 0E 11 01 1D 11 11 0E 08 06
0123 00 00 08 04 00 1E 11 11 11 1E 10 0E
0124 04 0A 00 11 11 11 1F 11 11 11 00 00
0125 00 00 08 15 01 0F 11 11 11 11 00 00
0126 00 00 00 11 3F 11 1F 11 11 11 00 00
0127 00 00 00 01 03 01 0F 11 11 11 00 00
0128 16 09 00 1F 04 04 04 04 04 1F 00 00
0129 00 00 16 09 00 06 04 04 04 1F 00 00
012A 00 0E 00 1F 04 04 04 04 04 1F 00 00
012B 00 00 00 0E 00 06 04 04 04 1F 00 00
012C 0A 04 00 1F 04 04 04 04 04 1F 00 00
012D 00 00 0A 04 00 06 04 04 04 1F 00 00
012E 00 00 00 1F 04 04 04 04 04 1F 04 18
012F 00 00 00 04 00 06 04 04 04 1F 04 18
0130 16 09 00 1F 04 04 04 04 04 1F 00 00
0131 00 00 00 00 00 06 04 04 04 1F 00 00
0132 00 00 00 17 12 12 12 12 12 0F 00 00
0133 00 00 00 12 00 1B 12 12 12 1F 10 0E
0134 04 0A 00 10 10 10 10 11 11 0E 00 00
0135 00 00 10 28 00 18 10 10 10 10 11 0E
0136 00 00 00 11 09 05 03 05 09 11 04 04
0137 00 00 00 01 01 11 09 07 09 11 04 04
0138 00 00 00 00 00 11 09 07 09 11 00 00
0139 08 04 00 01 01 01 01 01 01 1F 00 00
013A 08 04 00 1F 04 04 04 04 04 1F 00 00
013B 00 00 00 01 01 01 01 01 01 1F 08 06
013C 00 00 00 1F 04 04 04 04 04 1F 08 06
013D 00 00 00 11 11 09 01 01 01 1F 00 00
013E 00 00 00 13 12 0A 02 02 02 1C 00 00
013F 00 00 00 01 01 01 09 01 01 1F 00 00
0140 00 00 00 03 02 02 0A 02 02 1C 00 00
0141 00 00 00 01 01 01 03 01 01 1F 00 00
0142 00 00 00 03 02 02 06 03 02 1C 00 00
0143 08 04 00 11 11 13 15 19 11 11 00 00
0144 00 00 08 04 00 0F 11 11 11 11 00 00
0145 00 00 00 11 11 13 15 19 11 11 04 03
0146 00 00 00 00 00 0F 11 11 11 11 04 03
0147 0A 04 00 11 11 13 15 19 11 11 00 00
0148 00 00 0A 04 00 0F 11 11 11 11 00 00
0149 00 00 00 00 00 0F 11 11 11 11 00 00
014A 00 00 00 11 11 13 15 19 11 11 10 0C
014B 00 00 00 00 00 0F 11 11 11 11 10 0C
014C 00 0E 00 0E 11 11 11 11 11 0E 00 00
014D 00 00 00 0E 00 0E 11 11 11 0E 00 00
014E 0A 04 00 0E 11 11 11 11 11 0E 00 00
014F 00 00 0A 04 00 0E 11 11 11 0E 00 00
0150 14 0A 00 0E 11 11 11 11 11 0E 00 00
0151 00 00 14 0A 00 0E 11 11 11 0E 00 00
0152 00 00 00 1E 05 05 1D 05 05 1E 00 00
0153 00 00 00 00 00 0E 15 1D 05 0E 00 00
0154 08 04 00 0F 11 11 0F 11 11 11 00 00
0155 00 00 08 04 00 0D 13 01 01 01 00 00
0156 00 00 00 0F 11 11 0F 11 11 11 04 03
0157 00 00 00 00 00 0D 13 01 01 01 04 03
0158 0A 04 00 0F 11 11 0F 11 11 11 00 00
0159 00 00 0A 04 00 0D 13 01 01 01 00 00
015A 08 04 00 0E 11 01 0E 10 11 0E 00 00
015B 00 00 08 04 00 1E 01 0E 10 0F 00 00
015C 04 0A 00 0E 11 01 0E 10 11 0E 00 00
015D 00 00 04 0A 00 1E 01 0E 10 0F 00 00
015E 00 00 00 0E 11 01 0E 10 11 0E 04 03
015F 00 00 00 00 00 1E 01 0E 10 0F 04 03
0160 0A 04 00 0E 11 01 0E 10 11 0E 00 00
0161 00 00 0A 04 00 1E 01 0E 10 0F 00 00
0162 00 00 00 1F 04 04 04 04 04 04 04 03
0163 00 00 00 02 02 0F 02 02 02 1C 08 06
0164 0A 04 00 1F 04 04 04 04 04 04 00 00
0165 00 00 08 0A 02 0F 02 02 02 1C 00 00
0166 00 00 00 1F 04 0E 04 04 04 04 00 00
0167 00 00 00 02 0F 02 0F 02 02 1C 00 00
0168 16 09 00 11 11 11 11 11 11 0E 00 00
0169 00 00 16 09 00 11 11 11 11 1E 00 00
016A 00 0E 00 11 11 11 11 11 11 0E 00 00
016B 00 00 00 0E 00 11 11 11 11 1E 00 00
016C 0A 04 00 11 11 11 11 11 11 0E 00 00
016D 00 00 0A 04 00 11 11 11 11 1E 00 00
016E 0A 04 00 11 11 11 11 11 11 0E 00 00
016F 00 04 0A 04 00 11 11 11 11 1E 00 00
0170 14 0A 00 11 11 11 11 11 11 0E 00 00
0171 00 00 14 0A 00 11 11 11 11 1E 00 00
0172 00 00 00 11 11 11 11 11 11 0E 04 18
0173 00 00 00 00 00 11 11 11 11 1E 04 18
0174 04 0A 00 11 11 11 11 15 1B 11 00 00
0175 00 00 04 0A 00 11 11 15 15 0A 00 00
0176 04 0A 00 11 11 0A 04 04 04 04 00 00
0177 00 00 04 0A 00 11 11 11 11 1E 10 0E
0178 00 0A 00 11 11 0A 04 04 04 04 00 00
0179 08 04 00 1F 10 08 04 02 01 1F 00 00
017A 00 00 08 04 00 1F 08 04 02 1F 00 00
017B 00 04 00 1F 10 08 04 02 01 1F 00 00
017C 00 00 00 04 00 1F 08 04 02 1F 00 00
017D 0A 04 00 1F 10 08 04 02 01 1F 00 00
017E 00 00 0A 04 00 1F 08 04 02 1F 00 00
0401 00 0A 00 1F 01 01 0F 01 01 1F 00 00
0410 00 00 00 0E 11 11 11 1F 11 11 00 00
0411 00 00 00 1F 01 01 0F 11 11 0F 00 00
0412 00 00 00 0F 11 11 0F 11 11 0F 00 00
0413 00 00 00 1F 01 01 01 01 01 01 00 00
0414 00 00 00 0C 0A 0A 0A 0A 0A 1F 11 00
0415 00 00 00 1F 01 01 0F 01 01 1F 00 00
0416 00 00 00 15 15 15 0E 15 15 15 00 00
0417 00 00 00 0E 11 10 0E 10 11 0E 00 00
0418 00 00 00 11 11 19 15 13 11 11 00 00
0419 00 0A 04 11 11 19 15 13 11 11 00 00
041A 00 00 00 19 05 05 03 05 09 11 00 00
041B 00 00 00 1E 12 12 12 12 12 11 00 00
041C 00 00 00 11 1B 15 11 11 11 11 00 00
041D 00 00 00 11 11 11 1F 11 11 11 00 00
041E 00 00 00 0E 11 11 11 11 11 0E 00 00
041F 00 00 00 1F 11 11 11 11 11 11 00 00
0420 00 00 00 0F 11 11 0F 01 01 01 00 00
0421 00 00 00 0E 11 01 01 01 11 0E 00 00
0422 00 00 00 1F 04 04 04 04 04 04 00 00
0423 00 00 00 11 11 11 11 1E 10 0E 00 00
0424 00 00 00 04 0E 15 15 15 0E 04 00 00
0425 00 00 00 11 11 0A 04 0A 11 11 00 00
0426 00 00 00 00 09 09 09 09 09 1F 10 00
0427 00 00 00 11 11 11 1E 10 10 10 00 00
0428 00 00 00 15 15 15 15 15 15 1F 00 00
0429 00 00 00 15 15 15 15 15 15 1F 10 00
042A 00 00 00 00 03 02 0E 12 12 0E 00 00
042B 00 00 00 00 11 11 13 15 15 13 00 00
042C 00 00 00 00 01 01 0F 11 11 0F 00 00
042D 00 00 00 0E 11 10 1C 10 11 0E 00 00
042E 00 00 00 09 15 15 17 15 15 09 00 00
042F 00 00 00 00 1E 11 11 1E 11 11 00 00
0430 00 00 00 00 00 0E 10 1E 11 1E 00 00
0431 00 00 00 1E 01 0D 13 11 11 0E 00 00
0432 00 00 00 00 00 0F 11 0F 11 0F 00 00
0433 00 00 00 00 00 1F 01 01 01 01 00 00
0434 00 00 00 00 00 0C 0A 0A 0A 1F 11 00
0435 00 00 00 00 00 0E 11 1F 01 0E 00 00
0436 00 00 00 00 00 15 0E 04 0E 15 00 00
0437 00 00 00 00 00 06 09 04 09 06 00 00
0438 00 00 00 00 00 11 19 15 13 11 00 00
0439 00 00 00 0A 04 11 19 15 13 11 00 00
043A 00 00 00 00 00 11 09 07 09 11 00 00
043B 00 00 00 00 00 1E 12 12 12 11 00 00
043C 00 00 00 00 00 11 1B 15 11 11 00 00
043D 00 00 00 00 00 11 11 1F 11 11 00 00
043E 00 00 00 00 00 0E 11 11 11 0E 00 00
043F 00 00 00 00 00 1F 11 11 11 11 00 00
0440 00 00 00 00 00 0D 13 11 11 0F 01 01
0441 00 00 00 00 00 0E 11 01 11 0E 00 00
0442 00 00 00 00 00 1F 04 04 04 04 00 00
0443 00 00 00 00 00 11 11 11 11 1E 10 0E
0444 00 00 00 04 04 0E 15 15 15 0E 04 04
0445 00 00 00 00 00 11 0A 04 0A 11 00 00
0446 00 00 00 00 00 09 09 09 09 1F 10 00
0447 00 00 00 00 00 11 11 11 1E 10 00 00
0448 00 00 00 00 00 15 15 15 15 1F 00 00
0449 00 00 00 00 00 15 15 15 15 1F 10 00
044A 00 00 00 00 00 03 02 0E 12 0E 00 00
044B 00 00 00 00 00 11 11 13 15 13 00 00
044C 00 00 00 00 00 01 01 0F 11 0F 00 00
044D 00 00 00 00 00 0E 11 1C 11 0E 00 00
044E 00 00 00 00 00 09 15 17 15 09 00 00
044F 00 00 00 00 00 1E 11 11 1E 11 00 00
0451 00 00 00 0A 00 0E 11 1F 01 0E 00 00
2014 00 00 00 00 00 00 1F 00 00 00 00 00
2019 00 00 00 08 04 00 00 00 00 00 00 00
2026 00 00 00 00 00 00 00 00 15 15 00 00
20AC 00 00 00 0C 12 07 02 07 12 0C 00 00
2190 00 00 00 00 00 04 1E 1F 1E 04 00 00
2191 00 00 00 00 00 04 0E 1F 0E 0E 00 00
2192 00 00 00 00 00 04 0F 1F 0F 04 00 00
2193 00 00 00 00 00 0E 0E 1F 0E 04 00 00
0020 00 00 00 00 00 00 00 00 00 00 00 00
)";

} // namespace
