#pragma once

#include "Dodo.h"
#include "Texture.h"

#include "glm/ext/vector_float3.hpp"

namespace dodo
{
	namespace materials
	{
		struct standard
		{
			glm::vec3 color;
			float ambient, diffuse, specular, specular_alpha;
			std::shared_ptr<dodo::texture> texture, specular_map, environment_map;
		};

		struct pbr
		{
			// TODO
		};

		struct none
		{
		};
	}

	typedef std::variant<materials::standard, materials::pbr, materials::none> material;

	namespace color
	{
		static inline constexpr glm::vec3 WHITE = {1, 1, 1};
		static inline constexpr glm::vec3 GRAY = {0.5f, 0.5f, 0.5f};
		static inline constexpr glm::vec3 BLACK = {0, 0, 0};
		static inline constexpr glm::vec3 RED = {1, 0, 0};
		static inline constexpr glm::vec3 GREEN = {0, 1, 0};
		static inline constexpr glm::vec3 BLUE = {0, 0, 1};
		static inline constexpr glm::vec3 YELLOW = {1, 1, 0};
		static inline constexpr glm::vec3 MAGENTA = {1, 0, 1};
		static inline constexpr glm::vec3 CYAN = {0, 1, 1};
		static inline constexpr glm::vec3 ORANGE = {1, 0.5f, 0};
		static inline constexpr glm::vec3 PURPLE = {0.5f, 0, 0.5f};
		static inline constexpr glm::vec3 PINK = {1, 0.75f, 0.8f};
		static inline constexpr glm::vec3 BROWN = {0.6f, 0.4f, 0.2f};
	} // namespace color
}
