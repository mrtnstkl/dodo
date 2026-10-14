#pragma once

#include "Dodo.h"

namespace dodo
{

	/**
	 * \brief	Represents a point in 3d space.
	 */
	class point_entity
	{
	protected:
		glm::vec3 position_;

	public:
		point_entity(const glm::vec3& position = { 0, 0, 0 });

		virtual ~point_entity() = default;

		point_entity(const point_entity& other);

		point_entity& operator=(const point_entity& other);

		point_entity(point_entity&& other) noexcept = delete;

		point_entity& operator=(point_entity&& other) noexcept = delete;


		virtual const glm::vec3& position() const;

		virtual void set_position(const glm::vec3& position);


		virtual const glm::vec3& translate(const glm::vec3& relative_position);
	};


	/**
	 * \brief	Represents a rotatable and scalable object in 3d space.
	 */
	class entity : public point_entity
	{
	protected:
		glm::vec3 orientation_;
		glm::vec3 scaling_;

	public:
		entity(const glm::vec3& position = { 0, 0, 0 }, const glm::vec3& orientation = { 0, 0, 0 },
			const glm::vec3& scale = { 1, 1, 1 });

		virtual ~entity() override = default;

		entity(const entity& other);

		entity& operator=(const entity& other);

		entity(entity&& other) noexcept = delete;

		entity& operator=(entity&& other) noexcept = delete;


		virtual const glm::vec3& orientation() const;

		virtual const glm::vec3& scaling() const;


		virtual void set_orientation(const glm::vec3& orientation);

		virtual void set_scaling(const glm::vec3& scaling);


		virtual const glm::vec3& rotate(const glm::vec3& relative_orientation);

		virtual const glm::vec3& scale(const glm::vec3& relative_scale);
	};

}
