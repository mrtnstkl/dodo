#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <utility>
#include <glad/glad.h>

#include "Dodo.h"
#include "Util.h"

namespace dodo
{
	class texture;

	namespace shader
	{
		enum shader_type : uint8_t { vertex_shader, geometry_shader, fragment_shader };

		enum shader_options : uint8_t { file_path, source_code };

		/**
		 * \brief	Location definitions for shader input variables.
		 */
		namespace input_locations
		{
			constexpr GLint vertex_position = 0;
			constexpr GLint vertex_normal = 1;
			constexpr GLint vertex_texture_coordinate = 2;
			constexpr GLint custom = 3;
		}

		/**
		 * \brief	Location definitions for shader uniforms.
		 *			TODO: replace this
		namespace uniform_locations
		{
			constexpr GLint color = 0;
			constexpr GLint model_matrix = 1;
			constexpr GLint view_proj_matrix = 2;
			constexpr GLint camera_position = 3;

			constexpr GLint material_coefficients = 10; // ambient, diffuse, specular

			constexpr GLint directional_light_direction = 15;
			constexpr GLint directional_light_color = 16;
			constexpr GLint directional_light_array_size = 17;

			constexpr GLint point_light_position = 20;
			constexpr GLint point_light_color = 21;
			constexpr GLint point_light_attenuation = 22;
			constexpr GLint point_light_array_size = 23;

			constexpr GLint color_texture = 30;
			constexpr GLint specular_map = 31;
			constexpr GLint environment_map = 32;
		}
		 */


		/**
		 * \brief	Length definitions for uniform arrays in the shaders.
		 */
		namespace uniform_array_lengths
		{
			constexpr int directional_light = 8;
			constexpr int point_light = 8;
		}

		class shader_program;

		namespace uniform_structs
		{
			struct directional_light
			{
				static constexpr const char* name = "directional_light";

				const GLint direction, color;
				directional_light(const shader_program& shader, const char* struct_name);
			};

			struct point_light
			{
				static constexpr const char* name = "point_light";

				const GLint position, color, attenuation;
				point_light(const shader_program& shader, const char* struct_name);
			};
		}

		template <typename T>
		class uniform_struct_array
		{
			std::vector<T> elements_;
		public:
			uniform_struct_array() = default;

			uniform_struct_array(const shader_program& shader, const char* array_name, int array_length)
			{
				determine_locations(shader, array_name, array_length);
			}

			void determine_locations(const shader_program& shader, const char* array_name, int array_length)
			{
				elements_.clear();
				for (int i = 0; i < array_length; i++)
				{
					T t(shader, (std::string(array_name) + "[" + std::to_string(i) + "]").c_str());
					elements_.push_back(t);
				}
			}

			const T& operator[](size_t index) const
			{
				return elements_[index];
			}

			int size() const
			{
				return elements_.size();
			}
		};

		struct uniform_info
		{
			static constexpr size_t name_size = 64;

			GLuint program;
			GLuint index;
			GLsizei buf_size;
			GLsizei length;
			GLint size;
			GLenum type;
			GLchar name[name_size];
			GLint location;
		};

		struct uniform_locations
		{
			GLint material_color = -1;
			GLint model_matrix = -1;
			GLint view_proj_matrix = -1;
			GLint camera_position = -1;
			GLint material_coefficients = -1;
			GLint directional_light_direction = -1;
			GLint directional_light_color = -1;
			GLint directional_light_array_size = -1;
			GLint point_light_position = -1;
			GLint point_light_color = -1;
			GLint point_light_attenuation = -1;
			GLint point_light_array_size = -1;
			GLint color_texture = -1;
			GLint specular_map = -1;
			GLint environment_map = -1;

			void determine_locations(const shader_program& shader);
		};

		class shader
		{
			std::string source_;
			const std::string source_path_;
			const shader_type shader_type_;

			bool ready_ = false;
			GLuint handle_ = NULL;

			std::string error_str_;

		public:
			/**
			 * \brief	Constructs a wrapper for an opengl shader.
			 * \param shader_type	the shader type
			 * \param str			depending on the opt parameter, this should either be the
			 *						filepath to the shaders code, or the code itself
			 * \param opt			whether str will be interpreted as a filepath or as shader code
			 * \param compile		whether to compile the shader on construction
			 */
			shader(shader_type shader_type, std::string str, shader_options opt = file_path, bool compile = true);

			/**
			 * \brief	Deletes the shader.
			 */
			~shader();

			/**
			 * \brief	Copy constructor copies the source code, source path, shader type, and compiles the shader if 'other' is in a ready state.
			 */
			shader(const shader& other);

			shader(shader&& other) noexcept;

			shader& operator=(const shader& other) = delete;

			shader& operator=(shader&& other) = delete;


			/**
			 * \brief	Returns the opengl handle.
			 */
			GLuint handle() const;

			/**
			 * \brief	Returns true if the shader has been successfully compiled and is ready to use.
			 */
			bool ready() const;

			std::string error_str() const;


			/**
			 * \brief	Compiles the shader.
			 * \param read_file		whether to read the source code from the source file before compilation
			 * \return	true on success
			 */
			bool compile(bool read_file = true);

			/**
			 * \brief	Tells opengl to delete the shader.
			 *			Automatically called on object destruction.
			 */
			void destroy();

			/**
			 * \brief	Equivalent to destroy() + compile().
			 * \return	true on success
			 */
			bool recompile();


			/**
			 * \brief	Compiles an opengl shader.
			 * \param shader_type		the shader type
			 * \param source_code		the source code of the shader
			 * \param delete_on_error	if true, the function will delete the created shader if compilation failed, and return NULL
			 * \return	the opengl shader handle
			 */
			static GLuint compile_shader(shader_type shader_type, const std::string& source_code, bool delete_on_error);
		};

		class shader_program
		{
			std::shared_ptr<shader> vertex_shader_;
			std::shared_ptr<shader> fragment_shader_;
			std::shared_ptr<shader> geometry_shader_;

			bool ready_ = false;
			GLuint handle_ = NULL;

			uniform_locations uniform_locations_;
			std::unordered_map<std::size_t, uniform_info> uniforms_;

			uniform_struct_array<uniform_structs::directional_light> directional_light_locations_;
			uniform_struct_array<uniform_structs::point_light> point_light_locations_;

			void query_uniforms();

		public:
			/**
			 * \brief	Constructs a wrapper for an opengl shader program.
			 * \param vertex_shader		shared_ptr to the vertex shader
			 * \param fragment_shader	shared_ptr to the fragment shader
			 * \param compile			whether to compile the shader program on construction
			 * \param compile_children	whether to compile the vertex and fragment shaders on construction
			 */
			shader_program(std::shared_ptr<shader> vertex_shader,
			               std::shared_ptr<shader> fragment_shader,
			               bool compile = true,
			               bool compile_children = true);

			/**
			 * \brief	Constructs a wrapper for an opengl shader program.
			 * \param vertex_shader		shared_ptr to the vertex shader
			 * \param geometry_shader	shared_ptr to the geometry shader
			 * \param fragment_shader	shared_ptr to the fragment shader
			 * \param compile			whether to compile the shader program on construction
			 * \param compile_children	whether to compile the vertex, geometry and fragment shaders on construction
			 */
			shader_program(std::shared_ptr<shader> vertex_shader,
			               std::shared_ptr<shader> geometry_shader,
			               std::shared_ptr<shader> fragment_shader,
			               bool compile = true,
			               bool compile_children = true);

			/**
			 * \brief	Constructs a wrapper for an opengl shader program.
			 * \param vertex_shader		file path of the vertex shader
			 * \param fragment_shader	file path of the fragment shader
			 * \param compile			whether to compile the shader program on construction
			 * \param compile_children	whether to compile the vertex and fragment shaders on construction
			 */
			shader_program(const char* vertex_shader,
			               const char* fragment_shader,
			               bool compile = true,
			               bool compile_children = true);

			/**
			 * \brief	Constructs a wrapper for an opengl shader program.
			 * \param vertex_shader		file path of the vertex shader
			 * \param geometry_shader	file path of the geometry shader
			 * \param fragment_shader	file path of the fragment shader
			 * \param compile			whether to compile the shader program on construction
			 * \param compile_children	whether to compile the vertex, geometry and fragment shaders on construction
			 */
			shader_program(const char* vertex_shader,
			               const char* geometry_shader,
			               const char* fragment_shader,
			               bool compile = true,
			               bool compile_children = true);

			/**
			 * \brief	Deletes the shader program.
			 */
			~shader_program();

			/**
			 * \brief	Copy constructor copies the references to vertex and fragment shaders, and compiles them into a new shader program if 'other' is in a ready state.
			 */
			shader_program(const shader_program& other);

			/**
			 * \brief	Copy assignment copies the references to vertex and fragment shaders, and compiles them into a new shader program if 'other' is in a ready state.
			 */
			shader_program& operator=(const shader_program& other);

			shader_program(shader_program&& other) noexcept;

			shader_program& operator=(shader_program&& other) noexcept;


			/**
			 * \brief	Returns the opengl handle.
			 */
			[[nodiscard]] GLuint handle() const;

			/**
			 * \brief	Returns true if the shader program has been successfully compiled and is ready to use.
			 */
			[[nodiscard]] bool ready() const;

			[[nodiscard]] std::string error_msg() const;

			[[deprecated]] [[nodiscard]] bool has_uniform(GLint uniform_location) const;


			/**
			 * \brief	Returns a data structure containing the locations of all directional light uniform variables. \n
			 *			eg: "directional_light_locations()[2].color" gives the location of the color parameter
			 *			of the directional light at array index 2 in the shader.
			 */
			[[nodiscard]] const uniform_struct_array<uniform_structs::directional_light>& directional_light_locations() const;

			/**
			 * \brief	Returns a data structure containing the locations of all point light uniform variables. \n
			 *			eg: "point_light_locations()[2].position" gives the location of the position parameter
			 *			of the point light at array index 2 in the shader.
			 */
			[[nodiscard]] const uniform_struct_array<uniform_structs::point_light>& point_light_locations() const;


			[[nodiscard]] const uniform_locations& uf_locations() const;

			void print_debug_info(std::ostream& os) const;

			/**
			 * \brief	Tells opengl to use this shader program.
			 */
			void use() const;

			/**
			 * \brief	Creates the shader program from the vertex and fragment shaders.
			 * \param compile_children	whether to (re)compile the vertex and fragment shaders
			 * \return	true on success
			 */
			bool compile(bool compile_children = true);

			/**
			 * \brief	Tells opengl to delete the shader program.
			 * \param destroy_children	whether to delete the vertex and fragment shaders
			 */
			void destroy(bool destroy_children = false);

			/**
			 * \brief	Equivalent to destroy() + compile().
			 * \param recompile_children	whether to recompile the vertex and fragment shaders
			 * \return	true on success
			 */
			bool recompile(bool recompile_children = true);

			/**
			 * \brief	Returns the location of the uniform with the given name.
			 * \tparam UniformName	a string literal of the uniforms name
			 * \return	the uniforms location, or -1 if the uniform does not exist
			 */
			template<util::string_literal_wrapper UniformName>
			GLint uniform_location() const;

			/**
			 * \brief	NOTE: If you want to pass a string literal, use the templated version of this function. \n
			 *			Returns the location of the uniform with the given name.
			 * \param name	the name of the uniform
			 * \return	the uniforms location, or -1 if the uniform does not exist
			 */
			GLint uniform_location(const GLchar* name) const;

			/**
			 * \brief	Queries the shader program for a uniform location.
			 *			In most cases you should use uniform_location instead.
			 * \param name	the name of the uniform
			 * \return	the uniforms location
			 */
			GLint lookup_uniform_location(const GLchar* name) const;

			/**
			 * \brief	Queries the shader program for a uniform location.
			 *			In most cases you should use uniform_location instead.
			 * \param name	the name of the uniform
			 * \return	the uniforms location
			 */
			GLint lookup_uniform_location(const std::string& name) const;

			/**
			 * \brief	Creates an opengl shader program from a vertex and fragment shader.
			 * \param vertex_shader		the vertex shader
			 * \param fragment_shader	the fragment shader
			 * \return	the opengl handle of the shader program
			 */
			static GLuint create_shader_program(const shader& vertex_shader, const shader& fragment_shader);

			/**
			 * \brief	Creates an opengl shader program from a vertex and fragment shader.
			 * \param vertex_shader		the vertex shader
			 * \param fragment_shader	the fragment shader
			 * \param geometry_shader	the geometry shader
			 * \return	the opengl handle of the shader program
			 */
			static GLuint create_shader_program(const shader& vertex_shader, const shader& fragment_shader, const shader& geometry_shader);

			/**
			 * \brief	Creates an opengl shader program from a vertex and fragment shader.
			 * \param vertex_shader		the opengl handle of the vertex shader
			 * \param fragment_shader	the opengl handle of the fragment shader
			 * \param geometry_shader	the opengl handle of the geometry shader. this is optional
			 * \return	the opengl handle of the shader program
			 */
			static GLuint create_shader_program(GLuint vertex_shader, GLuint fragment_shader, GLuint geometry_shader = NULL);


			// Sets the value of a float uniform using glUniform1f
			void set_uniform(GLint location, GLfloat f) const;
			// Sets the value of a vec2 uniform using glUniform2f
			void set_uniform(GLint location, GLfloat f1, GLfloat f2) const;
			// Sets the value of a vec2 uniform using glUniform2fv
			void set_uniform(GLint location, const glm::vec2& vec) const;
			// Sets the value of a vec3 uniform using glUniform3f
			void set_uniform(GLint location, GLfloat f1, GLfloat f2, GLfloat f3) const;
			// Sets the value of a vec3 uniform using glUniform3fv
			void set_uniform(GLint location, const glm::vec3& vec) const;
			// Sets the value of a vec4 uniform using glUniform4f
			void set_uniform(GLint location, GLfloat f1, GLfloat f2, GLfloat f3, GLfloat f4) const;
			// Sets the value of a vec4 uniform using glUniform4fv
			void set_uniform(GLint location, const glm::vec4& vec) const;
			// Sets the value of a double uniform using glUniform1d
			void set_uniform(GLint location, GLdouble d) const;
			// Sets the value of a dvec2 uniform using glUniform2d
			void set_uniform(GLint location, GLdouble d1, GLdouble d2) const;
			// Sets the value of a dvec2 uniform using glUniform2dv
			void set_uniform(GLint location, const glm::dvec2& vec) const;
			// Sets the value of a dvec3 uniform using glUniform3d
			void set_uniform(GLint location, GLdouble d1, GLdouble d2, GLdouble d3) const;
			// Sets the value of a dvec3 uniform using glUniform3dv
			void set_uniform(GLint location, const glm::dvec3& vec) const;
			// Sets the value of a dvec4 uniform using glUniform4d
			void set_uniform(GLint location, GLdouble d1, GLdouble d2, GLdouble d3, GLdouble d4) const;
			// Sets the value of a dvec4 uniform using glUniform4dv
			void set_uniform(GLint location, const glm::dvec4& vec) const;
			// Sets the value of a int uniform using glUniform1i
			void set_uniform(GLint location, GLint i) const;
			// Sets the value of a ivec2 uniform using glUniform2i
			void set_uniform(GLint location, GLint i1, GLint i2) const;
			// Sets the value of a ivec2 uniform using glUniform2iv
			void set_uniform(GLint location, const glm::ivec2& vec) const;
			// Sets the value of a ivec3 uniform using glUniform3i
			void set_uniform(GLint location, GLint i1, GLint i2, GLint i3) const;
			// Sets the value of a ivec3 uniform using glUniform3iv
			void set_uniform(GLint location, const glm::ivec3& vec) const;
			// Sets the value of a ivec4 uniform using glUniform4i
			void set_uniform(GLint location, GLint i1, GLint i2, GLint i3, GLint i4) const;
			// Sets the value of a ivec4 uniform using glUniform4iv
			void set_uniform(GLint location, const glm::ivec4& vec) const;
			// Sets the value of a unsigned int uniform using glUniform1ui
			void set_uniform(GLint location, GLuint i) const;
			// Sets the value of a uvec2 uniform using glUniform2ui
			void set_uniform(GLint location, GLuint i1, GLuint i2) const;
			// Sets the value of a uvec3 uniform using glUniform3ui
			void set_uniform(GLint location, GLuint i1, GLuint i2, GLuint i3) const;
			// Sets the value of a uvec4 uniform using glUniform4ui
			void set_uniform(GLint location, GLuint i1, GLuint i2, GLuint i3, GLuint i4) const;
			// Sets the value of a mat3 uniform using glUniformMatrix3fv
			void set_uniform(GLint location, const glm::mat3& mat) const;
			// Sets the value of a mat4 uniform using glUniformMatrix4fv
			void set_uniform(GLint location, const glm::mat4& mat) const;
			// Sets the value of a mat3 uniform using glUniformMatrix3fv
			void set_uniform(GLint location, const glm::dmat3& mat) const;
			// Sets the value of a mat4 uniform using glUniformMatrix4fv
			void set_uniform(GLint location, const glm::dmat4& mat) const;
			// Sets the value of a texture uniform and binds that texture to the given unit
			void set_uniform(GLint location, const dodo::texture& texture, int tex_unit) const;

			template <dodo::util::string_literal_wrapper UniformName, typename... Args>
			void set_uniform(Args&&... args) const;
		};

	}
}


template <dodo::util::string_literal_wrapper UniformName>
GLint dodo::shader::shader_program::uniform_location() const
{
	const auto it = uniforms_.find(util::constexpr_hash<UniformName>());
	return it != uniforms_.end() ? it->second.location : -1;
}

template <dodo::util::string_literal_wrapper UniformName, typename... Args>
void dodo::shader::shader_program::set_uniform(Args&&... args) const
{
	set_uniform(uniform_location<UniformName>(), std::forward<Args>(args)...);
}
