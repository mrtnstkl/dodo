#pragma once
#include <sstream>
#include <string>
#include <algorithm>

#include "Dodo.h"

namespace dodo
{
	namespace util
	{
		/**
		 * \brief	Reads the entire content of a file as a string.
		 *			On error, an empty string is returned.
		 * \param file_path		path to the file that should be read
		 * \return	the file content
		 */
		std::string read_file(const std::string& file_path);

		std::string format_gl_debug_output(GLenum source, GLenum type, GLuint id, GLenum severity, const char* msg);

		glm::vec3 spherical_to_cartesian(float azimuth, float zenith, float radius);


		template <size_t N>
		struct string_literal_wrapper
		{
			const size_t n = N;
			char str[N];
			constexpr string_literal_wrapper(const char(&str)[N]);
		};

		constexpr std::size_t constexpr_hash(const std::string_view str) noexcept;

		template <string_literal_wrapper Str>
		consteval std::size_t constexpr_hash() noexcept;
	}
}


template <size_t N>
constexpr dodo::util::string_literal_wrapper<N>::string_literal_wrapper(const char(&str)[N])
{
	std::copy_n(str, N, this->str);
}

constexpr std::size_t dodo::util::constexpr_hash(const std::string_view str) noexcept
{
	size_t result = 0;
	for (const char c : str)
	{
		result = (result * 31) ^ c;
	}
	return result;
}

template <dodo::util::string_literal_wrapper Str>
consteval std::size_t dodo::util::constexpr_hash() noexcept
{
	size_t result = 0;
	for (size_t i = 0; i < Str.n - 1; ++i)
	{
		result = (result * 31) ^ Str.str[i];
	}
	return result;
}
