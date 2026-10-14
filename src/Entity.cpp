#include "Entity.h"


const glm::vec3& dodo::point_entity::position() const
{
	return position_;
}

dodo::point_entity::point_entity(const glm::vec3& position)
	: position_(position)
{
}

dodo::point_entity::point_entity(const point_entity& other)
	: position_(other.position_)
{
}

dodo::point_entity& dodo::point_entity::operator=(const dodo::point_entity& other)
{
	if (this == &other)
		return *this;
	position_ = other.position_;
	return *this;
}


void dodo::point_entity::set_position(const glm::vec3& position)
{
	position_ = position;
}

const glm::vec3& dodo::point_entity::translate(const glm::vec3& relative_position)
{
	position_.x += relative_position.x;
	position_.y += relative_position.y;
	position_.z += relative_position.z;
	return position_;
}

dodo::entity::entity(const glm::vec3& position, const glm::vec3& orientation, const glm::vec3& scale)
	: dodo::point_entity(position), orientation_(orientation), scaling_(scale)
{
}

dodo::entity::entity(const entity& other)
	: dodo::point_entity(other),
	  orientation_(other.orientation_),
	  scaling_(other.scaling_)
{
}

dodo::entity& dodo::entity::operator=(const entity& other)
{
	if (this == &other)
		return *this;
	dodo::point_entity::operator =(other);
	orientation_ = other.orientation_;
	scaling_ = other.scaling_;
	return *this;
}

const glm::vec3& dodo::entity::orientation() const
{
	return orientation_;
}

const glm::vec3& dodo::entity::scaling() const
{
	return scaling_;
}

void dodo::entity::set_orientation(const glm::vec3& orientation)
{
	orientation_ = orientation;
}


void dodo::entity::set_scaling(const ::glm::vec3& scaling)
{
	scaling_ = scaling;
}

const glm::vec3& dodo::entity::rotate(const glm::vec3& relative_orientation)
{
	orientation_ += relative_orientation;
	return orientation_;
}

const glm::vec3& dodo::entity::scale(const ::glm::vec3& relative_scale)
{
	scaling_ += relative_scale;
	return scaling_;
}
