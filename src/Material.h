#pragma once

#include "Dodo.h"
#include "Texture.h"

namespace dodo
{
	namespace materials
	{
		struct standard
		{
			glm::vec3 color;
			float ambient, diffuse, specular, specular_alpha;

			// todo
			std::shared_ptr<dodo::texture> texture, specular_map, environment_map;
		};

		struct pbr
		{
		};

		struct none
		{
		};
	}

	typedef std::variant<materials::standard, materials::pbr, materials::none> material;
}
