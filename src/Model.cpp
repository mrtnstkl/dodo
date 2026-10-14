#include "Model.h"

#include "Shader.h"

void dodo::model::calculate_model_matrix()
{
	model_transform_matrix_ =
		glm::scale(
			glm::rotate( // yaw
				glm::rotate( // pitch
					glm::rotate( // roll
						glm::translate(glm::mat4(1), position_),
						orientation_.z, {0, 0, 1}),
					orientation_.y, {0, 1, 0}),
				orientation_.x, {1, 0, 0}),
			scaling_);
}

dodo::model::model(dodo::mesh_base* mesh, const glm::vec3& position, const dodo::material& material,
                  const glm::vec3& orientation, const glm::vec3& scale)
	: entity(position, orientation, scale), mesh_(std::move(mesh)), material_(material)
{
	calculate_model_matrix();
}

dodo::model::model(const model& other)
	: entity(other),
	  model_transform_matrix_(other.model_transform_matrix_),
	  mesh_(other.mesh_),
	  material_(other.material_)
{
}

dodo::model::model(model&& other) noexcept
	: entity(other),
	  model_transform_matrix_(other.model_transform_matrix_),
	  mesh_(other.mesh_),
	  material_(other.material_)
{
	other.mesh_ = nullptr;
}

dodo::model& dodo::model::operator=(const model& other)
{
	if (this == &other)
		return *this;
	entity::operator =(other);
	model_transform_matrix_ = other.model_transform_matrix_;
	mesh_ = other.mesh_;
	material_ = other.material_;
	return *this;
}

dodo::model& dodo::model::operator=(model&& other) noexcept
{
	if (this == &other)
		return *this;
	entity::operator =(other);
	model_transform_matrix_ = other.model_transform_matrix_;
	mesh_ = other.mesh_;
	other.mesh_ = nullptr;
	material_ = other.material_;
	return *this;
}

dodo::model::~model()
{
	delete mesh_;
}


void dodo::model::set_mesh(dodo::mesh_base* mesh)
{
	delete mesh_;
	mesh_ = mesh;
}

void dodo::model::set_material(material material)
{
	material_ = material;
}


dodo::mesh_base& dodo::model::mesh()
{
	return *mesh_;
}


void dodo::model::draw(GLenum mode) const
{
	mesh_->draw(mode);
}

void dodo::model::draw_points() const
{
	mesh_->draw_points();
}

void dodo::model::set_position(const glm::vec3& position)
{
	entity::set_position(position);
	calculate_model_matrix();
}

const glm::vec3& dodo::model::translate(const glm::vec3& relative_position)
{
	auto& r = entity::translate(relative_position);
	calculate_model_matrix();
	return r;
}

void dodo::model::set_orientation(const glm::vec3& orientation)
{
	dodo::entity::set_orientation(orientation);
	calculate_model_matrix();
}

void dodo::model::set_scaling(const glm::vec3& scaling)
{
	dodo::entity::set_scaling(scaling);
	calculate_model_matrix();
}

const glm::vec3& dodo::model::rotate(const glm::vec3& relative_orientation)
{
	auto& r = dodo::entity::rotate(relative_orientation);
	calculate_model_matrix();
	return r;
}

const glm::vec3& dodo::model::scale(const glm::vec3& relative_scale)
{
	auto& r = dodo::entity::scale(relative_scale);
	calculate_model_matrix();
	return r;
}
