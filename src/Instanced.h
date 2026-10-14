#pragma once

#include "Dodo.h"
#include "Model.h"

typedef unsigned int uint;

namespace dodo::instanced
{
	namespace array_structs
	{
		struct instanced_array_struct
		{
		protected:
			instanced_array_struct() = default;
		public:
			static void bind(GLuint starting_index) = delete;
		};

		struct transformation : instanced_array_struct
		{
			glm::mat4 trans_mat;

			transformation(glm::mat4 trans_mat = {});
			static void bind(GLuint starting_index);
		};

		struct position : instanced_array_struct
		{
			glm::vec3 pos_vec;

			position(glm::vec3 pos_vec = {});
			static void bind(GLuint starting_index);
		};
	}

	template <typename T>
	class instanced_model : public model
	{
		static_assert(std::is_base_of_v<array_structs::instanced_array_struct, T>,
			"Type parameter must be derived from instanced_array_struct");

		GLuint instance_vbo_handle_ = NULL;

		std::vector<T> instance_array_;

		uint draw_amount_ = 1;

		void init();

	public:

		instanced_model(dodo::mesh_base* mesh = {},
		                const glm::vec3& position = { 0, 0, 0 },
		                const dodo::material& material = dodo::materials::standard{},
		                const glm::vec3& orientation = { 0, 0, 0 },
		                const glm::vec3& scale = { 1, 1, 1 }
		);

		instanced_model(const instanced_model& other);

		instanced_model(instanced_model&& other) noexcept;

		instanced_model& operator=(const instanced_model& other);

		instanced_model& operator=(instanced_model&& other) noexcept;

		~instanced_model() override;

		/**
		 * \brief	Returns the size of the instance-array.
		 */
		uint array_size() const;

		/**
		 * \brief	Resizes the instance array.
		 * \param elements	how many elements (=instances) the array should contain
		 */
		void set_array_size(uint elements);

		/**
		 * \brief	Defines how many instances should be drawn.
		 * \param amount	maximum amount of instances to draw (draw
		 *					amount is limited by size of instance-array)
		 */
		void set_draw_amount(uint amount);

		/**
		 * \brief	Copies instance array data to the GPU, to make changes visible.
		 */
		void update_instance_buffer();

		/**
		 * \brief	Draws the model instances.
		 */
		void draw(GLenum ignored) const override;

		/**
		 * \brief	Returns instance at a given index.
		 * \param index	the index of the instance to return.
		 *				[ index < array_size() ]
		 */
		T& operator[](uint index);

		/**
		 * \brief	Returns iterator, to iterate over instances.
		 */
		typename std::vector<T>::iterator begin();

		/**
		 * \brief	End iterator.
		 */
		typename std::vector<T>::iterator end();
	};


	template <typename T>
	void instanced_model<T>::init()
	{
		if (instance_vbo_handle_ != NULL)
		{
			return;
		}
		glGenBuffers(1, &instance_vbo_handle_);
		glBindVertexArray(model::mesh_->vao_handle());
		glBindBuffer(GL_ARRAY_BUFFER, instance_vbo_handle_);
		T::bind(model::mesh_->get_free_input_location());
		glBindBuffer(GL_ARRAY_BUFFER, NULL);
		glBindVertexArray(NULL);
	}

	template <typename T>
	instanced_model<T>::instanced_model(dodo::mesh_base* mesh, const glm::vec3& position, const dodo::material& material, const glm::vec3& orientation,
	                                    const glm::vec3& scale)
		: model(mesh, position, material, orientation, scale)
	{
		init();
	}

	template <typename T>
	instanced_model<T>::instanced_model(const instanced_model& other)
		: model(other), instance_array_(other.instance_array_), draw_amount_(other.draw_amount_)
	{
		init();
		update_instance_buffer();
	}

	template <typename T>
	instanced_model<T>::instanced_model(instanced_model&& other) noexcept
		: model(std::move(other)), instance_vbo_handle_(other.instance_vbo_handle_),
		  instance_array_(std::move(other.instance_array_)), draw_amount_(other.draw_amount_)
	{
		other.instance_vbo_handle_ = NULL;
	}

	template <typename T>
	instanced_model<T>& instanced_model<T>::operator=(const instanced_model& other)
	{
		if (this == &other)
			return *this;
		model::operator =(other);
		instance_array_ = other.instance_array_;
		draw_amount_ = other.draw_amount_;
		init();
		update_instance_buffer();
		return *this;
	}

	template <typename T>
	instanced_model<T>& instanced_model<T>::operator=(instanced_model&& other) noexcept
	{
		if (this == &other)
			return *this;
		model::operator =(std::move(other));
		instance_vbo_handle_ = other.instance_vbo_handle_;
		other.instance_vbo_handle_ = NULL;
		instance_array_ = std::move(other.instance_array_);
		draw_amount_ = other.draw_amount_;
		return *this;
	}

	template <typename T>
	instanced_model<T>::~instanced_model()
	{
		glDeleteBuffers(1, &instance_vbo_handle_);
	}

	template <typename T>
	uint instanced_model<T>::array_size() const
	{
		return instance_array_.size();
	}


	template <typename T>
	void instanced_model<T>::set_array_size(uint elements)
	{
		instance_array_.resize(elements);
	}

	template <typename T>
	void instanced_model<T>::set_draw_amount(uint amount)
	{
		draw_amount_ = amount;
	}

	template <typename T>
	void instanced_model<T>::update_instance_buffer()
	{
		glBindBuffer(GL_ARRAY_BUFFER, instance_vbo_handle_);
		glBufferData(GL_ARRAY_BUFFER, sizeof(T) * instance_array_.size(), instance_array_.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, NULL);
	}

	template <typename T>
	void instanced_model<T>::draw(GLenum ignored) const
	{
		model::mesh_->draw_instanced(instance_array_.size() < draw_amount_ ? instance_array_.size() : draw_amount_);
	}

	template <typename T>
	T& instanced_model<T>::operator[](uint index)
	{
		return instance_array_[index];
	}

	template <typename T>
	typename std::vector<T>::iterator instanced_model<T>::begin()
	{
		return instance_array_.begin();
	}

	template <typename T>
	typename std::vector<T>::iterator instanced_model<T>::end()
	{
		return instance_array_.end();
	}


}
