#pragma once

#include <algorithm>
#include <functional>
#include <span>
#include <vector>
#include "Dodo.h"

namespace dodo
{
	namespace vertex_structs
	{
		template <typename T>
		concept is_vertex_struct = requires(T & t, GLuint i)
		{
			T{};
			T::configure_vao(i, i, i);
			(GLuint)T::first_free_input_location;
		};

		struct vertex3_norm_uv
		{
			float x, y, z;
			float norm_x, norm_y, norm_z;
			float tex_u, tex_v;
			static void configure_vao(GLuint vao, GLuint vbo, GLuint vib);
			static constexpr GLuint first_free_input_location = 3;
		};

		struct vertex3
		{
			float x, y, z;
			static void configure_vao(GLuint vao, GLuint vbo, GLuint vib);
			static constexpr GLuint first_free_input_location = 1;
		};
	}

	class mesh_base
	{
	private:
		/**
		 * \brief	Creates the necessary opengl objects (vao, vbo, vib).
		 */
		void create_buffers();

	protected:
		GLuint vao_handle_ = NULL;
		GLuint vbo_handle_ = NULL;
		GLuint vib_handle_ = NULL;

		uint32_t vertex_count_ = 0;
		uint32_t index_count_ = 0;

		/**
		 * \brief	Configures the vertex-array-object.
		 */
		virtual void configure_vao() const = 0;

		mesh_base();

	public:

		/**
		 * \brief	Deletes opengl buffers and the vao.
		 */
		virtual ~mesh_base();

		mesh_base(const mesh_base& other);

		mesh_base(mesh_base&& other) noexcept;

		mesh_base& operator=(const mesh_base& other);

		mesh_base& operator=(mesh_base&& other) noexcept;


		size_t vertex_count() const
		{
			return vertex_count_;
		}

		/**
		 * @brief	Low level function for manually setting the vertex count.
		 * @param new_vertex_count	the new value for the vertex count
		 */
		void set_vertex_count(uint32_t new_vertex_count)
		{
			vertex_count_ = new_vertex_count;
		}

		/**
		 * @brief	Low level function for manually setting the index count.
		 * @param new_index_count	the new value for the index count
		 */
		void set_index_count(uint32_t new_index_count)
		{
			index_count_ = new_index_count;
		}

		/**
		 * \brief	Draws the mesh.
		 * \param mode	the opengl drawing mode
		 */
		void draw(GLenum mode = GL_TRIANGLES) const;

		// todo
		void draw_triangles() const;

		/**
		 * \brief	Draws the mesh in GL_POINTS mode.
		 */
		void draw_points() const;

		/**
		 * \brief	Makes an instanced draw-call.
		 * \param amount	the amount of instances to draw
		 */
		void draw_instanced(int amount) const;

		/**
		 * \brief	Writes the mesh data to the vertex and index buffers.
		 *			This needs to be called to make mesh changes visible.
		 */
		virtual void write_buffers() = 0;

		virtual GLuint get_free_input_location() = 0;

		GLuint vao_handle() const;
		GLuint vbo_handle() const;
		GLuint vib_handle() const;

		using buffer_flag = uint8_t;
		struct buffer_flags
		{
			static constexpr buffer_flag vertex_buffer = 1 << 0;
			static constexpr buffer_flag index_buffer = 1 << 1;
		};
	};

	template <typename Vertex> requires vertex_structs::is_vertex_struct<Vertex>
	class gpu_only_mesh : public dodo::mesh_base
	{
		void configure_vao() const override
		{
			Vertex::configure_vao(vao_handle_, vbo_handle_, vib_handle_);
		}

	public:

		/**
		 * \brief	Constructs a primitive mesh that only wraps a gpu buffer. No vertex data is stored in main memory.
		 * \param vertex_count	how many vertices the buffer will hold
		 */
		explicit gpu_only_mesh(size_t vertex_count, size_t index_count = 0)
			: mesh_base()
		{
			gpu_only_mesh<Vertex>::configure_vao();
			vertex_count_ = vertex_count;
			// todo: index count
		}

		~gpu_only_mesh() override = default;

		gpu_only_mesh(const gpu_only_mesh& other)
			: mesh_base(other)
		{
			gpu_only_mesh<Vertex>::configure_vao();
		}

		gpu_only_mesh(gpu_only_mesh&& other) noexcept
			: mesh_base(other)
		{
			gpu_only_mesh<Vertex>::configure_vao();
		}

		gpu_only_mesh& operator=(const gpu_only_mesh& other)
		{
			return *mesh_base::operator=(other);
		}

		gpu_only_mesh& operator=(gpu_only_mesh&& other) noexcept
		{
			return* mesh_base::operator=(std::forward<gpu_only_mesh>(other));
		}


		GLuint get_free_input_location() override
		{
			return Vertex::first_free_input_location;
		}

		/**
		 * \brief	Initializes the vertex buffer.
		 */
		void write_buffers() override
		{
			glBindBuffer(GL_ARRAY_BUFFER, vbo_handle_);
			glBufferData(GL_ARRAY_BUFFER, vertex_count_ * sizeof(Vertex), NULL, GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, NULL);
		}

		/**
		 * \brief	Initializes the vertex buffer.
		 * \param data	the data that the vertex buffer will be initialized with
		 */
		void write_buffers(const Vertex* data)
		{
			glBindBuffer(GL_ARRAY_BUFFER, vbo_handle_);
			glBufferData(GL_ARRAY_BUFFER, vertex_count_ * sizeof(Vertex), data, GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, NULL);
		}

		/**
		 * \brief	Copies some data into a section of the vertex buffer.
		 * \param offset	byte offset from the beginning of the buffer
		 * \param size		amount of bytes to be copied
		 * \param data		the data to copy from
		 */
		void write_buffers_section(GLintptr offset, GLsizeiptr size, const Vertex* data)
		{
			glBindBuffer(GL_ARRAY_BUFFER, vbo_handle_);
			glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
			glBindBuffer(GL_ARRAY_BUFFER, NULL);
		}
	};

	template <typename Vertex> requires vertex_structs::is_vertex_struct<Vertex>
	class mesh : public mesh_base
	{
	protected:
		std::vector<Vertex> vertices_;
		std::vector<uint32_t> indices_;

		/**
		 * \brief	Configures the vertex-array-object.
		 */
		void configure_vao() const override
		{
			Vertex::configure_vao(vao_handle_, vbo_handle_, vib_handle_);
		}

	public:
		/**
		 * \brief	Constructs a new mesh object with empty vertex data.
		 */
		mesh()
			: mesh_base()
		{
			mesh<Vertex>::configure_vao();
		}

		~mesh() override = default;

		/**
		 * \brief	Copy constructor creates a new mesh by copying all vertex data.
		 */
		mesh(const mesh& other)
			: mesh_base(other),
			vertices_(other.vertices_),
			indices_(other.indices_)
		{
			mesh<Vertex>::configure_vao();
			write_buffers();
		}

		/**
		 * \brief	Copy assignment creates a new mesh by copying all vertex data.
		 */
		mesh& operator=(const mesh& other)
		{
			mesh_base::operator=(other);
			vertices_ = other.vertices_;
			indices_ = other.indices_;
			write_buffers();
			return *this;
		}

		mesh(mesh&& other) noexcept
			: mesh_base(std::move(other)),
			vertices_(std::move(other.vertices_)),
			indices_(std::move(other.indices_))
		{
		}

		mesh& operator=(mesh&& other) noexcept
		{
			mesh_base::operator=(other);
			vertices_ = std::move(other.vertices_);
			indices_ = std::move(other.indices_);
			return *this;
		}

		/**
		 * \brief	Adds a vertex to the mesh.
		 * \return	the index of the new vertex
		 */
		uint32_t add_vertex(const Vertex& vertex)
		{
			vertices_.push_back(vertex);
			vertex_count_ = vertices_.size();
			return vertices_.size() - 1;
		}

		Vertex& get_vertex(size_t i)
		{
			return vertices_[i];
		}

		std::span<Vertex> vertex_view()
		{
			return vertices_;
		}

		std::span<uint32_t> index_view()
		{
			return indices_;
		}

		/**
		 * @brief	Low level function for modifying the vertex buffer.
		 * @note	If vector size is changed, make sure to update the vertex count using set_vertex_count()!
		 * @return 	the data buffer that gets uploaded to the GPU
		 */
		std::vector<Vertex>& vertices()
		{
			return vertices_;
		}

		/**
		 * @brief	Low level function for modifying the index buffer.
		 * @note	If vector size is changed, make sure to update the index count using set_index_count()!
		 * @return 	the data buffer that gets uploaded to the GPU
		 */
		std::vector<uint32_t>& indices()
		{
			return indices_;
		}

		/**
		 * @brief	Helper function for sorting the vertex buffer.
		 * @note	Only useful for meshes that don't use the index buffer (i.e. point meshes).
		 * @param compare_func	the comparison function
		 */
		void sort_vertices(std::function<bool(const Vertex&, const Vertex&)> compare_func)
		{
			std::sort(vertices_.begin(), vertices_.end(), compare_func);
		}

		/**
		 * \brief	Adds a triangle to the mesh.
		 * \param v1	index of the first vertex
		 * \param v2	index of the second vertex
		 * \param v3	index of the third vertex
		 */
		void add_triangle(uint32_t v1, uint32_t v2, uint32_t v3)
		{
			indices_.push_back(v1);
			indices_.push_back(v2);
			indices_.push_back(v3);

			index_count_ = indices_.size();
		}

		/**
		 * \brief	Allocates and initializes the vertex and index buffers.
		 * 			This also needs to be called when the vertex or index count of the mesh changes.
		 * \param vertex_buffer_usage	the expected usage pattern of the vertex buffer. if NULL, no vertex buffer will be allocated
		 * \param index_buffer_usage	the expected usage pattern of the index buffer. if NULL, no index buffer will be allocated
		 */
		virtual void allocate_buffers(GLenum vertex_buffer_usage = GL_STATIC_DRAW, GLenum index_buffer_usage = GL_STATIC_DRAW)
		{
			if (vertex_buffer_usage != NULL)
			{
				glBindBuffer(GL_ARRAY_BUFFER, vbo_handle_);
				glBufferData(GL_ARRAY_BUFFER, vertex_count_ * sizeof(Vertex), NULL, vertex_buffer_usage);
				glBindBuffer(GL_ARRAY_BUFFER, NULL);
			}
			if (vertex_buffer_usage != NULL)
			{
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vib_handle_);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count_ * sizeof(unsigned int), NULL, vertex_buffer_usage);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);
			}
		}

		/**
		 * \brief	Writes mesh data to the vertex and index buffers.
		 *			This needs to be called to make mesh changes visible.
		 *			Buffers must have already been created using `write_buffers` or `allocate_buffers`
		 * \param buffers	the buffers whose data should be updated
		 */
		virtual void update_buffer_data(buffer_flag buffers = buffer_flags::vertex_buffer | buffer_flags::index_buffer)
		{
			if (buffers & buffer_flags::vertex_buffer)
			{
				glBindBuffer(GL_ARRAY_BUFFER, vbo_handle_);
				GLint buffer_size = 0;
				glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &buffer_size);
				glBufferSubData(GL_ARRAY_BUFFER, 0, std::min<GLint>(buffer_size, vertex_count_ * sizeof(Vertex)), vertices_.data());
				glBindBuffer(GL_ARRAY_BUFFER, NULL);
			}
			if (buffers & buffer_flags::index_buffer)
			{
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vib_handle_);
				GLint buffer_size = 0;
				glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &buffer_size);
				glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, std::min<GLint>(buffer_size, index_count_ * sizeof(unsigned int)), indices_.data());
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);
			}
		}

		/**
		 * @brief	Equivalent to allocate_buffers(GL_STATIC_DRAW, GL_STATIC_DRAW) + update_buffer_data()
		 */
		void write_buffers() override
		{
			// set VBO data
			glBindBuffer(GL_ARRAY_BUFFER, vbo_handle_);
			glBufferData(GL_ARRAY_BUFFER, vertex_count_ * sizeof(Vertex), vertices_.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, NULL);

			// set VIO data
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vib_handle_);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count_ * sizeof(unsigned int), indices_.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);
		}

		GLuint get_free_input_location() override
		{
			return Vertex::first_free_input_location;
		}

		/**
		 * \brief	Flips the vertex-order of all triangles of the mesh.
		 */
		void flip_winding_order()
		{
			for (int i = 0; i + 2 < indices_.size(); i += 3)
			{
				const uint32_t temp = indices_[i];
				indices_[i] = indices_[i + 2];
				indices_[i + 2] = temp;
			}
			write_buffers();
		}

	};

	using basic_mesh = mesh<vertex_structs::vertex3>;

	class standard_mesh : public mesh<vertex_structs::vertex3_norm_uv>
	{
	public:
		/**
		 * \brief	Automatically calculates vertex normals.
		 */
		void generate_normals();

		/**
		 * \brief	Creates a cube mesh.
		 * \param width		width of the cube
		 * \param height	height of the cube
		 * \param depth		depth of the cube
		 * \return	the created mesh
		 */
		static standard_mesh generate_cube(float width, float height, float depth);

		/**
		 * \brief	Creates a cylinder mesh.
		 * \param radius	radius of the cylinder
		 * \param height	height of the cylinder
		 * \param segments	amount of segments. minimum is 3
		 * \return	the created mesh
		 */
		static standard_mesh generate_cylinder(float radius, float height, uint32_t segments);

		/**
		 * \brief	Creates a sphere mesh.
		 * \param radius					radius of the sphere
		 * \param longitudinal_segments		number of longitudinal segments. minimum is 3
		 * \param latitudinal_segments		number of latitudinal segments minimum is 2
		 * \return	the created mesh
		 */
		static standard_mesh generate_sphere(float radius, uint32_t longitudinal_segments, uint32_t latitudinal_segments);

		/**
		 * \brief	Creates a 2d rect facing the positive y-axis.
		 * \param height	the height of the mesh (z direction).
		 * \param width		the width of the mesh (x direction).
		 * \return	the created mesh
		 */
		static standard_mesh generate_2d_rect(float height, float width);

		/**
		 * \brief	Reads an obj-file and constructs a mesh from it.
		 * \param path				the path to the file
		 * \param generate_normals	whether vertex normals should be calculated.
		 *							if true, all normal definitions from the file will be ignored
		 * \return	the created mesh
		 */
		static standard_mesh from_obj_file(const char* path, bool generate_normals = false);

		void write_to_obj(const char *path) const;
	};

}
