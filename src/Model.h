#pragma once

#include "Dodo.h"
#include "Entity.h"
#include "Material.h"
#include "Mesh.h"
#include "Texture.h"

namespace dodo
{
	class model : public entity
	{
	protected:
		friend class renderer;

		glm::mat4 model_transform_matrix_;

		mesh_base* mesh_;

		material material_;

		void calculate_model_matrix();

	public:

		/**
		 * \brief	Constructs a model.
		 * \param mesh			the 3d-mesh. model takes ownership of it
		 * \param position		the position in 3d space
		 * \param material		the material parameters of the model
		 * \param texture		the texture that will be bound when calling draw()
		 * \param spec_map		the texture that will be bound as a specular map when calling draw()
		 * \param orientation	the orientation (yaw, pitch, roll)
		 * \param scale			the size/scale
		 */
		model(mesh_base* mesh,
		      const glm::vec3& position = {0, 0, 0},
		      const material& material = materials::standard{},
		      const glm::vec3& orientation = {0, 0, 0},
		      const glm::vec3& scale = {1, 1, 1}
		);

		model(const model& other);
		model(model&& other) noexcept;
		model& operator=(const model& other);
		model& operator=(model&& other) noexcept;

		virtual ~model() override;


		void set_mesh(mesh_base* mesh);

		void set_material(material material);

		mesh_base& mesh();

		/**
		 * \brief	Draws the model.
		 * \param mode	the openGL drawing mode
		 */
		virtual void draw(GLenum mode = GL_TRIANGLES) const;

		/**
		 * \brief	Draws the models vertices.
		 */
		virtual void draw_points() const;


		// overrides for automatically recalculating the model matrix:

		void set_position(const glm::vec3 & position) override;

		void set_orientation(const glm::vec3 & orientation) override;

		void set_scaling(const glm::vec3 & scaling) override;

		const glm::vec3& translate(const glm::vec3 & relative_position) override;

		const glm::vec3& rotate(const glm::vec3 & relative_orientation) override;

		const glm::vec3& scale(const glm::vec3 & relative_scale) override;

	};
}
