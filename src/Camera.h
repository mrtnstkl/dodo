#pragma once

#include <memory>
#include "Entity.h"

namespace dodo
{


	class camera : public point_entity
	{
	protected:
		glm::mat4 camera_matrix_ = glm::mat4(1);
		glm::vec3 view_vector_ = {0, 0, -1};
		glm::vec3 up_vector_ = {0, 1, 0};

	public:
		[[nodiscard]] virtual const glm::mat4& camera_matrix() const;
		[[nodiscard]] virtual const glm::vec3& view_vector() const;

		virtual float pitch() const = 0;
		virtual float yaw() const = 0;

		virtual void pitch(float angle) = 0;
		virtual void yaw(float angle) = 0;
		virtual void look_at(const glm::vec3& target) = 0;

		virtual void set_pitch(float angle) = 0;
		virtual void set_yaw(float angle) = 0;
		virtual void set_near(float near) = 0;
		virtual void set_far(float far) = 0;
		virtual void set_up_vector(const glm::vec3& up_vector = {0, 1, 0}) = 0;
		virtual void set_aspect_ratio(float ratio) = 0;
		virtual void set_aspect_ratio(float width, float height) = 0;
		virtual void set_position(const glm::vec3& position) override = 0;

		virtual const glm::vec3& translate(const glm::vec3& relative_position) override = 0;
	};


	class camera_basic_impl : public camera
	{
	protected:
		float near_ = .01f, far_ = 1000.f;
		float pitch_ = 0, yaw_ = 0;
		float aspect_ratio_ = 1;

		camera_basic_impl() = default;
		virtual void update() = 0;
	public:
		float near() const;
		float far() const;
		float aspect_ratio() const;
		float pitch() const override;
		float yaw() const override;
		[[deprecated]] void pitch(float angle) override;
		[[deprecated]] void yaw(float angle) override;
		void look_at(const glm::vec3& target) override;
		void set_pitch(float angle) override;
		void set_yaw(float angle) override;
		void set_near(float near) override;
		void set_far(float far) override;
		void set_up_vector(const glm::vec3& up_vector) override;
		void set_aspect_ratio(float ratio) override;
		void set_aspect_ratio(float width, float height) override;
		void set_position(const glm::vec3& position) override;
		const glm::vec3& translate(const glm::vec3& relative_position) override;
		~camera_basic_impl() override = default;
	};

	class perspective_camera : public camera_basic_impl
	{
		float fov_ = 90.f;

		glm::mat4 view_matrix_ = glm::mat4(1);
		glm::mat4 proj_matrix_ = glm::mat4(1);

		void update() override;

	public:
		perspective_camera(const glm::vec3& position = { 0, 0, 0 },
			const glm::vec3& up_vector = { 0, 1, 0 },
			float fov = 90.f, float near = .1f, float far = 100.f,
			float aspect_ratio = 1.f);

		~perspective_camera() override = default;

		float fov() const;
		void set_fov(float fov);
	};

	class orthogonal_camera : public camera_basic_impl
	{
		float width_, height_;

		void update() override;

	public:
		orthogonal_camera(glm::vec3 position, float width = 1, float height = 1, float near = .1f, float far = 100.f);
		~orthogonal_camera() override = default;

		void set_aspect_ratio(float ratio) override;
		void set_aspect_ratio(float width, float height) override;
		void set_view_dimensions(float width, float height);
	};

	class arcball_camera_controller : public camera
	{
		std::shared_ptr<camera> base_;
		//glm::vec3 orbit_center_ = {0,0,0};

		float azimuthal_angle_ = 0;
		float polar_angle_ = 0;
		float distance_ = 10;

	public:
		explicit arcball_camera_controller(std::shared_ptr<camera> base);

		void set_distance(float distance);

		[[nodiscard]] camera& camera() const;
		[[nodiscard]] float distance() const;

		float pitch() const override;
		float yaw() const override;
		void pitch(float angle) override;
		void yaw(float angle) override;
		void look_at(const glm::vec3& target) override;
		void set_pitch(float angle) override;
		void set_yaw(float angle) override;
		void set_near(float near) override;
		void set_far(float far) override;
		void set_up_vector(const glm::vec3& up_vector) override;
		void set_aspect_ratio(float ratio) override;
		void set_aspect_ratio(float width, float height) override;
		void set_position(const glm::vec3& position) override;
		const glm::vec3& translate(const glm::vec3& relative_position) override;
		~arcball_camera_controller() override;

		[[nodiscard]] const glm::mat4& camera_matrix() const override;
		[[nodiscard]] const glm::vec3& view_vector() const override;
	};

}
