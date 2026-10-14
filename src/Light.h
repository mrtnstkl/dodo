#pragma once

#include "Dodo.h"
#include "Entity.h"

namespace dodo
{
	class light
	{
	protected:
		glm::vec3 color_;

		explicit light(const glm::vec3& color = { 1, 1, 1 });

	public:
		const glm::vec3& color() const;
	};


	class directional_light : public light
	{
	private:
		glm::vec3 direction_;

	public:
		explicit directional_light(const glm::vec3& direction, const glm::vec3& color = { 1,1,1 });

		const glm::vec3& direction() const;
	};



	class point_light : public light, public point_entity
	{
	private:
		glm::vec3 attenuation_;

	public:
		point_light(const glm::vec3& position = { 0, 0, 0 }, const glm::vec3& color = { 1, 1, 1 },
			const glm::vec3& attenuation = { 1, 1, 1 });

		const glm::vec3& attenuation() const;

	};


}
