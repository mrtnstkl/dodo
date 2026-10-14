#include "Light.h"

dodo::light::light(const glm::vec3& color)
	: color_(color)
{
}

const glm::vec3& dodo::light::color() const
{
	return color_;
}

dodo::directional_light::directional_light(const glm::vec3& direction, const glm::vec3& color)
	: light(color), direction_(direction)
{
}

const glm::vec3& dodo::directional_light::direction() const
{
	return direction_;
}

dodo::point_light::point_light(const glm::vec3& position, const glm::vec3& color, const glm::vec3& attenuation)
	: light(color), point_entity(position), attenuation_(attenuation)
{
}

const glm::vec3& dodo::point_light::attenuation() const
{
	return attenuation_;
}
