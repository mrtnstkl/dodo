#pragma once

#include <climits>
#include <vector>
#include "Dodo.h"
#include "Mesh.h"
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
	} // namespace array_structs


	template <typename T>
	class instanced_array
	{
		static_assert(std::is_base_of_v<array_structs::instanced_array_struct, T>,
					  "Type parameter must be derived from instanced_array_struct");

	protected:
		GLuint instance_vbo_handle_ = NULL;
		std::vector<T> instance_array_;

	public:
		instanced_array(size_t size = 1);

		instanced_array(const instanced_array<T> &other);
		instanced_array(instanced_array<T> &&other) noexcept;
		instanced_array &operator=(const instanced_array<T> &other);
		instanced_array &operator=(instanced_array<T> &&other) noexcept;
		virtual ~instanced_array();

		void bind(mesh_base *mesh);

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
		 * \brief	Copies instance array data to the GPU, to make changes visible.
		 */
		void update_instance_buffer();

		/**
		 * \brief	Returns instance at a given index.
		 * \param index	the index of the instance to return.
		 *				[ index < array_size() ]
		 */
		T &operator[](uint index);

		/**
		 * \brief	Returns a reference to the instance array.
		 */
		std::vector<T>& instance_data();

		/**
		 * \brief	Returns iterator, to iterate over instances.
		 */
		typename std::vector<T>::iterator begin();

		/**
		 * \brief	End iterator.
		 */
		typename std::vector<T>::iterator end();
	};


	template <typename V, typename I>
	class instanced_mesh : public mesh<V>, public instanced_array<I>
	{
		uint draw_amount_ = UINT_MAX;

	public:
		instanced_mesh(size_t instance_count = 1);
		instanced_mesh(mesh<V> &&mesh, size_t instance_count = 1);
		instanced_mesh(const instanced_mesh<V, I> &other);
		instanced_mesh(instanced_mesh<V, I> &&other) noexcept;
		instanced_mesh &operator=(const instanced_mesh<V, I> &other);
		instanced_mesh &operator=(instanced_mesh<V, I> &&other) noexcept;
		~instanced_mesh() override = default;

		void draw(GLenum ignored = NULL) const override;
		/**
		 * \brief	Defines how many instances should be drawn.
		 * \param amount	maximum amount of instances to draw (draw
		 *					amount is limited by size of instance-array)
		 */
		void set_draw_amount(uint amount);
	};


	template <typename T>
	instanced_array<T>::instanced_array(size_t size) : instance_array_(size)
	{
	}

	template <typename T>
	instanced_array<T>::instanced_array(const instanced_array<T> &other)
	{
		instance_array_ = other.instance_array_;
	}
	template <typename T>
	instanced_array<T>::instanced_array(instanced_array<T> &&other) noexcept
	{
		instance_vbo_handle_ = other.instance_vbo_handle_;
		instance_array_ = std::move(other.instance_array_);
		other.instance_vbo_handle_ = NULL;
	}
	template <typename T>
	instanced_array<T> &instanced_array<T>::operator=(const instanced_array<T> &other)
	{
		instance_array_ = other.instance_array_;
		return *this;
	}
	template <typename T>
	instanced_array<T> &instanced_array<T>::operator=(instanced_array<T> &&other) noexcept
	{
		instance_vbo_handle_ = other.instance_vbo_handle_;
		instance_array_ = std::move(other.instance_array_);
		other.instance_vbo_handle_ = NULL;
		return *this;
	}
	template <typename T>
	instanced_array<T>::~instanced_array()
	{
		if (instance_vbo_handle_ != NULL)
		{
			glDeleteBuffers(1, &instance_vbo_handle_);
		}
	}

	template <typename T>
	void instanced_array<T>::bind(mesh_base *mesh)
	{
		if (instance_vbo_handle_ != NULL)
		{
			return;
		}
		glGenBuffers(1, &instance_vbo_handle_);
		glBindVertexArray(mesh->vao_handle());
		glBindBuffer(GL_ARRAY_BUFFER, instance_vbo_handle_);
		T::bind(mesh->get_free_input_location());
		glBindBuffer(GL_ARRAY_BUFFER, NULL);
		glBindVertexArray(NULL);
	}

	template <typename T>
	uint instanced_array<T>::array_size() const
	{
		return instance_array_.size();
	}

	template <typename T>
	void instanced_array<T>::set_array_size(uint elements)
	{
		instance_array_.resize(elements);
	}

	template <typename T>
	void instanced_array<T>::update_instance_buffer()
	{
		glBindBuffer(GL_ARRAY_BUFFER, instance_vbo_handle_);
		glBufferData(GL_ARRAY_BUFFER, sizeof(T) * instance_array_.size(), instance_array_.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, NULL);
	}

	template <typename T>
	T &instanced_array<T>::operator[](uint index)
	{
		return instance_array_[index];
	}

	template <typename T>
	std::vector<T>& instanced_array<T>::instance_data()
	{
		return instance_array_;
	}

	template <typename T>
	typename std::vector<T>::iterator instanced_array<T>::begin()
	{
		return instance_array_.begin();
	}

	template <typename T>
	typename std::vector<T>::iterator instanced_array<T>::end()
	{
		return instance_array_.end();
	}


	template <typename V, typename I>
	instanced_mesh<V, I>::instanced_mesh(size_t instance_count) : dodo::mesh<V>(), instanced_array<I>(instance_count)
	{
		instanced_array<I>::bind(this);
	}
	template <typename V, typename I>
	instanced_mesh<V, I>::instanced_mesh(mesh<V> &&mesh, size_t instance_count)
		: dodo::mesh<V>(std::move(mesh)), instanced_array<I>(instance_count)
	{
		instanced_array<I>::bind(this);
	}
	template <typename V, typename I>
	instanced_mesh<V, I>::instanced_mesh(const instanced_mesh<V, I> &other)
		: dodo::mesh<V>(other), instanced_array<I>(other)
	{
		draw_amount_ = other.draw_amount_;
		instanced_array<I>::bind(this);
	}
	template <typename V, typename I>
	instanced_mesh<V, I>::instanced_mesh(instanced_mesh<V, I> &&other) noexcept
		: dodo::mesh<V>(std::move(other)), instanced_array<I>(std::move(other))
	{
		draw_amount_ = other.draw_amount_;
	}
	template <typename V, typename I>
	instanced_mesh<V, I> &instanced_mesh<V, I>::operator=(const instanced_mesh<V, I> &other)
	{
		draw_amount_ = other.draw_amount_;
		dodo::mesh<V>::operator=(other);
		instanced_array<I>::operator=(other);
		return *this;
	}
	template <typename V, typename I>
	instanced_mesh<V, I> &instanced_mesh<V, I>::operator=(instanced_mesh<V, I> &&other) noexcept
	{
		draw_amount_ = other.draw_amount_;
		dodo::mesh<V>::operator=(std::move(other));
		instanced_array<I>::operator=(std::move(other));
		return *this;
	}

	template <typename V, typename I>
	void instanced_mesh<V, I>::draw(GLenum ignored) const
	{
		dodo::mesh<V>::draw_instanced(instanced_array<I>::instance_array_.size() < draw_amount_
						   ? instanced_array<I>::instance_array_.size()
						   : draw_amount_);
	}

	template <typename V, typename I>
	void instanced_mesh<V, I>::set_draw_amount(uint amount)
	{
		draw_amount_ = amount;
	}

} // namespace dodo::instanced


// =============================== DEPRECATED ===============================

namespace dodo::instanced
{

	// deprecated: use a normal model with instanced_mesh instead
	template <typename T>
	class [[deprecated]] instanced_model : public model
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
