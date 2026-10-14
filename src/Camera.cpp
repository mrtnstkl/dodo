#include "Camera.h"
#include <algorithm>
#include "glm/fwd.hpp"


dodo::arcball_camera_controller::arcball_camera_controller(std::shared_ptr<dodo::camera> base)
	: base_(base)
{
	set_position(base_->camera_matrix()[3]);
	set_pitch(base_->pitch() + glm::half_pi<float>());
	set_yaw(base_->yaw() + glm::half_pi<float>());
}

void dodo::arcball_camera_controller::set_distance(float distance)
{
	distance_ = distance > 0.0f ? distance : 0.0f;
	base_->set_position(position_ - distance_ * base_->view_vector());
}

dodo::camera& dodo::arcball_camera_controller::camera() const
{
	return *base_;
}

float dodo::arcball_camera_controller::distance() const
{
	return distance_;
}

float dodo::arcball_camera_controller::pitch() const
{
	return polar_angle_;
}

float dodo::arcball_camera_controller::yaw() const
{
	return azimuthal_angle_;
}

void dodo::arcball_camera_controller::pitch(float angle)
{
	set_pitch(polar_angle_ + angle);
}

void dodo::arcball_camera_controller::yaw(float angle)
{
	set_yaw(azimuthal_angle_ + angle);
}

void dodo::arcball_camera_controller::look_at(const glm::vec3& target)
{
	// todo
}

void dodo::arcball_camera_controller::set_pitch(float angle)
{
	polar_angle_ = std::clamp(angle, .1f, glm::pi<float>() - .1f);
	base_->set_pitch(polar_angle_ - glm::half_pi<float>());
	base_->set_position(position_ - distance_ * base_->view_vector());
}

void dodo::arcball_camera_controller::set_yaw(float angle)
{
	azimuthal_angle_ = angle;
	base_->set_yaw(azimuthal_angle_ - glm::half_pi<float>());
	base_->set_position(position_ - distance_ * base_->view_vector());
}

void dodo::arcball_camera_controller::set_near(float near)
{
	base_->set_near(near);
}

void dodo::arcball_camera_controller::set_far(float far)
{
	base_->set_far(far);
}

void dodo::arcball_camera_controller::set_up_vector(const glm::vec3& up_vector)
{
	base_->set_up_vector(up_vector);
}

void dodo::arcball_camera_controller::set_aspect_ratio(float ratio)
{
	base_->set_aspect_ratio(ratio);
}

void dodo::arcball_camera_controller::set_aspect_ratio(float width, float height)
{
	base_->set_aspect_ratio(width, height);
}

void dodo::arcball_camera_controller::set_position(const glm::vec3& position)
{
	position_ = position;
	base_->set_position(position_ - distance_ * base_->view_vector());
}

const glm::vec3& dodo::arcball_camera_controller::translate(const glm::vec3& relative_position)
{
	position_ += relative_position;
	return base_->translate(relative_position);
}

dodo::arcball_camera_controller::~arcball_camera_controller()
{
}

const glm::mat4& dodo::arcball_camera_controller::camera_matrix() const
{
	return base_->camera_matrix();
}

const glm::vec3& dodo::arcball_camera_controller::view_vector() const
{
	return base_->view_vector();
}


const glm::mat4& dodo::camera::camera_matrix() const
{
	return camera_matrix_;
}

const glm::vec3& dodo::camera::view_vector() const
{
	return view_vector_;
}

float dodo::camera_basic_impl::near() const
{
	return near_;
}

float dodo::camera_basic_impl::far() const
{
	return far_;
}

float dodo::camera_basic_impl::aspect_ratio() const
{
	return aspect_ratio_;
}

float dodo::camera_basic_impl::pitch() const
{
	return pitch_;
}

float dodo::camera_basic_impl::yaw() const
{
	return yaw_;
}

void dodo::camera_basic_impl::pitch(float angle)
{
	pitch_ += angle;
	update();
}

void dodo::camera_basic_impl::yaw(float angle)
{
	yaw_ += angle;
	update();
}

void dodo::camera_basic_impl::look_at(const glm::vec3& target)
{
	const auto view_vec = glm::normalize(target - position_);
	yaw_ = atan2f(-view_vec.x, view_vec.z);
	pitch_ = asinf(view_vec.y);
	update();
}

void dodo::camera_basic_impl::set_pitch(float angle)
{
	pitch_ = angle;
	update();
}

void dodo::camera_basic_impl::set_yaw(float angle)
{
	yaw_ = angle;
	update();
}

void dodo::camera_basic_impl::set_near(float near)
{
	near_ = near;
	update();
}

void dodo::camera_basic_impl::set_far(float far)
{
	far_ = far;
	update();
}

void dodo::camera_basic_impl::set_up_vector(const glm::vec3& up_vector)
{
	up_vector_ = up_vector;
	update();
}

void dodo::camera_basic_impl::set_aspect_ratio(float ratio)
{
	aspect_ratio_ = ratio;
	update();
}

void dodo::camera_basic_impl::set_aspect_ratio(float width, float height)
{
	aspect_ratio_ = width / height;
	update();
}

void dodo::camera_basic_impl::set_position(const glm::vec3& position)
{
	position_ = position;
	update();
}

const glm::vec3& dodo::camera_basic_impl::translate(const glm::vec3& relative_position)
{
	position_ += relative_position;
	update();
	return position_;
}

void dodo::perspective_camera::update()
{
	view_vector_ = {
		cosf(pitch_) * -sinf(yaw_),
		sinf(pitch_),
		cosf(pitch_) * cosf(yaw_)
	};
	camera_matrix_ =
		glm::perspective(glm::radians(fov_), aspect_ratio_, near_, far_) *
		glm::lookAt(position_, position_ + view_vector_, up_vector_);
}

dodo::perspective_camera::perspective_camera(const glm::vec3& position,
                                            const glm::vec3& up_vector, float fov, float near, float far,
                                            float aspect_ratio)
	: fov_(fov)
{
	position_ = position;
	aspect_ratio_ = aspect_ratio;
	far_ = far;
	near_ = near;

	up_vector_ = up_vector;
	perspective_camera::update();
}

float dodo::perspective_camera::fov() const
{
	return fov_;
}

void dodo::perspective_camera::set_fov(float fov)
{
	fov_ = fov;
	update();
}

void dodo::orthogonal_camera::update()
{
	camera_matrix_ =
		glm::lookAt(position_, view_vector_ - position_, up_vector_) *
		glm::ortho(
			position_.x - width_ * .5f, position_.x + width_ * .5f,
			position_.y - height_ * .5f, position_.y + height_ * .5f,
			position_.z - near_, position_.z - far_
		);
}

dodo::orthogonal_camera::orthogonal_camera(glm::vec3 position, float width, float height, float near, float far)
{
	position_ = position;
	near_ = near;
	far_ = far;
	orthogonal_camera::set_aspect_ratio(width, height);
	set_view_dimensions(width, height);
}

void dodo::orthogonal_camera::set_aspect_ratio(float ratio)
{
	aspect_ratio_ = ratio;
	width_ = height_ * ratio;
	update();
}

void dodo::orthogonal_camera::set_aspect_ratio(float width, float height)
{
	set_aspect_ratio(width / height);
}

void dodo::orthogonal_camera::set_view_dimensions(float width, float height)
{
	if (width == 0.f)
	{
		height_ = height;
		width_ = aspect_ratio_ * height;
	}
	else if (height == 0.f)
	{
		width_ = width;
		height_ = width / aspect_ratio_;
	}
	else
	{
		height_ = height;
		width_ = width;
		aspect_ratio_ = width / height;
	}
	update();
}
