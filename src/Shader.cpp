#include "Shader.h"

#include <algorithm>
#include <utility>

#include "Texture.h"


static GLenum shader_type_to_gl_enum(const dodo::shader::shader_type st)
{
	switch (st)
	{
	case dodo::shader::vertex_shader: return GL_VERTEX_SHADER;
	case dodo::shader::geometry_shader: return GL_GEOMETRY_SHADER;
	case dodo::shader::fragment_shader: return GL_FRAGMENT_SHADER;
	default: return NULL;
	}
}

#define GET_UNIFORM_STRUCT_MEMBER_LOCATION(x) \
	x ( shader.lookup_uniform_location(std::string(struct_name) + "." + #x) )

dodo::shader::uniform_structs::directional_light::directional_light(const shader_program& shader, const char* struct_name)
	: GET_UNIFORM_STRUCT_MEMBER_LOCATION(direction),
	GET_UNIFORM_STRUCT_MEMBER_LOCATION(color)
{
}

dodo::shader::uniform_structs::point_light::point_light(const shader_program& shader, const char* struct_name)
	: GET_UNIFORM_STRUCT_MEMBER_LOCATION(position),
	GET_UNIFORM_STRUCT_MEMBER_LOCATION(color),
	GET_UNIFORM_STRUCT_MEMBER_LOCATION(attenuation)
{
}


#define GET_UNIFORM_LOCATION(x) \
	x = shader.lookup_uniform_location(#x)

void dodo::shader::uniform_locations::determine_locations(const shader_program& shader)
{
	GET_UNIFORM_LOCATION(material_color);
	GET_UNIFORM_LOCATION(model_matrix);
	GET_UNIFORM_LOCATION(view_proj_matrix);
	GET_UNIFORM_LOCATION(camera_position);
	GET_UNIFORM_LOCATION(material_coefficients);
	GET_UNIFORM_LOCATION(directional_light_direction);
	GET_UNIFORM_LOCATION(directional_light_color);
	GET_UNIFORM_LOCATION(directional_light_array_size);
	GET_UNIFORM_LOCATION(point_light_position);
	GET_UNIFORM_LOCATION(point_light_color);
	GET_UNIFORM_LOCATION(point_light_attenuation);
	GET_UNIFORM_LOCATION(point_light_array_size);
	GET_UNIFORM_LOCATION(color_texture);
	GET_UNIFORM_LOCATION(specular_map);
	GET_UNIFORM_LOCATION(environment_map);
}

dodo::shader::shader::shader(shader_type shader_type, std::string str, shader_options opt, bool compile)
	: source_(opt == source_code ? std::move(str) : ""), source_path_(opt == file_path ? std::move(str) : ""),
	  shader_type_(shader_type)
{
	if (compile)
	{
		this->compile();
	}
}

dodo::shader::shader::~shader()
{
	destroy();
}

dodo::shader::shader::shader(const shader& other)
	: source_(other.source_),
	source_path_(other.source_path_),
	shader_type_(other.shader_type_)
{
	if (other.ready_)
	{
		compile(false);
	}
}

dodo::shader::shader::shader(shader&& other) noexcept
	: source_(std::move(other.source_)),
	source_path_(other.source_path_),
	shader_type_(other.shader_type_),
	ready_(other.ready_),
	handle_(other.handle_)
{
	other.handle_ = NULL;
}


GLuint dodo::shader::shader::handle() const
{
	return handle_;
}

bool dodo::shader::shader::ready() const
{
	return ready_;
}

std::string dodo::shader::shader::error_str() const
{
	return error_str_;
}

bool dodo::shader::shader::compile(bool read_file)
{
	if (!source_path_.empty() && read_file)
	{
		source_ = util::read_file(source_path_);
		// todo: check that reading was successful
	}
	if (ready_)
	{
		destroy();
	}

	handle_ = compile_shader(shader_type_, source_, false);

	GLint compile_status;
	glGetShaderiv(handle_, GL_COMPILE_STATUS, &compile_status);
	if (compile_status != GL_TRUE)
	{
		// compilation failed
		GLsizei log_length = 0;
		GLchar log[1024];
		glGetShaderInfoLog(handle_, 1024, &log_length, log);
		error_str_ = std::string(log, log_length);

		glDeleteShader(handle_);
		handle_ = NULL;
		ready_ = false;
		return false;
	}
	error_str_.clear();

	ready_ = true;
	return true;
}

void dodo::shader::shader::destroy()
{
	ready_ = false;

	if (handle_ != NULL)
	{
		glDeleteShader(handle_);
		handle_ = NULL;
	}
}

bool dodo::shader::shader::recompile()
{
	destroy();
	return compile();
}

GLuint dodo::shader::shader::compile_shader(const shader_type shader_type, const std::string& source_code, bool delete_on_error)
{
	const GLuint shader_id = glCreateShader(shader_type_to_gl_enum(shader_type));

	const char* src = source_code.c_str();
	glShaderSource(shader_id, 1, &src, nullptr);

	glCompileShader(shader_id);

	if (delete_on_error)
	{
		int compile_status;
		glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compile_status);
		if (compile_status == GL_FALSE)
		{
			glDeleteShader(shader_id);
			return NULL;
		}
	}

	return shader_id;
}

void dodo::shader::shader_program::query_uniforms()
{
	uniform_locations_.determine_locations(*this);

	uniforms_.clear();

	GLint active_uniforms;
	glGetProgramiv(handle_, GL_ACTIVE_UNIFORMS, &active_uniforms);

	uniforms_.clear();
	for (int i = 0; i < active_uniforms; i++)
	{
		uniform_info info{};
		glGetActiveUniform(handle_, i, uniform_info::name_size, &info.length, &info.size, &info.type, info.name);
		info.location = glGetUniformLocation(handle_, info.name);
		uniforms_[util::constexpr_hash(info.name)] = info;
	}
}


dodo::shader::shader_program::shader_program(std::shared_ptr<shader> vertex_shader,
                                            std::shared_ptr<shader> fragment_shader,
                                            const bool compile, const bool compile_children)
	: shader_program(std::move(vertex_shader), nullptr, std::move(fragment_shader), compile, compile_children)
{
}

dodo::shader::shader_program::shader_program(std::shared_ptr<shader> vertex_shader,
                                            std::shared_ptr<shader> geometry_shader,
                                            std::shared_ptr<shader> fragment_shader,
                                            bool compile, bool compile_children)
	: vertex_shader_(std::move(vertex_shader)), fragment_shader_(std::move(fragment_shader)),
	  geometry_shader_(std::move(geometry_shader))
{
	assert(vertex_shader_ != nullptr);
	assert(fragment_shader_ != nullptr);
	if (compile)
	{
		this->compile(compile_children);
	}
}

dodo::shader::shader_program::shader_program(const char* vertex_shader,
                                            const char* fragment_shader,
                                            bool compile, bool compile_children)
	: shader_program(
		std::make_shared<shader>(shader_type::vertex_shader, vertex_shader, file_path, false),
		std::make_shared<shader>(shader_type::fragment_shader, fragment_shader, file_path, false),
		compile, compile_children)
{
}

dodo::shader::shader_program::shader_program(const char* vertex_shader,
                                            const char* geometry_shader,
                                            const char* fragment_shader,
                                            bool compile, bool compile_children)
	: shader_program(
		std::make_shared<shader>(shader_type::vertex_shader, vertex_shader, file_path, false),
		std::make_shared<shader>(shader_type::geometry_shader, geometry_shader, file_path, false),
		std::make_shared<shader>(shader_type::fragment_shader, fragment_shader, file_path, false),
		compile, compile_children)
{
}

dodo::shader::shader_program::~shader_program()
{
	destroy(false);
}

dodo::shader::shader_program::shader_program(const shader_program& other)
	: vertex_shader_(other.vertex_shader_),
	  fragment_shader_(other.fragment_shader_),
	  geometry_shader_(other.geometry_shader_)
{
	if (other.ready_)
	{
		compile(false);
	}
}

dodo::shader::shader_program::shader_program(shader_program&& other) noexcept
	: vertex_shader_(std::move(other.vertex_shader_)),
	  fragment_shader_(std::move(other.fragment_shader_)),
	  geometry_shader_(std::move(other.geometry_shader_)),
	  ready_(other.ready_),
	  handle_(other.handle_)
{
	other.handle_ = NULL;
}

dodo::shader::shader_program& dodo::shader::shader_program::operator=(const shader_program& other)
{
	if (this == &other)
	{
		return *this;
	}
	vertex_shader_ = other.vertex_shader_;
	fragment_shader_ = other.fragment_shader_;
	geometry_shader_ = other.geometry_shader_;
	if (other.ready_)
	{
		compile(false);
	}
	return *this;
}

dodo::shader::shader_program& dodo::shader::shader_program::operator=(shader_program&& other) noexcept
{
	if (this == &other)
	{
		return *this;
	}
	vertex_shader_ = std::move(other.vertex_shader_);
	fragment_shader_ = std::move(other.fragment_shader_);
	geometry_shader_ = std::move(other.geometry_shader_);
	ready_ = other.ready_;
	handle_ = other.handle_;
	other.handle_ = NULL;
	return *this;
}

GLuint dodo::shader::shader_program::handle() const
{
	return handle_;
}

bool dodo::shader::shader_program::ready() const
{
	return ready_;
}

std::string dodo::shader::shader_program::error_msg() const
{
	std::string str;

	char error_log[1024];
	if (!vertex_shader_->ready())
	{
		str += "Error in vertex shader: ";
		str += vertex_shader_->error_str();
		str += '\n';
	}
	if (!fragment_shader_->ready())
	{
		str += "Error in fragment shader: ";
		str += fragment_shader_->error_str();
		str += '\n';
	}
	if (geometry_shader_ != nullptr && !geometry_shader_->ready())
	{
		str += "Error in geometry shader: ";
		str += geometry_shader_->error_str();
		str += '\n';
	}

	return str;
}

bool dodo::shader::shader_program::has_uniform(GLint uniform_location) const
{
	return false;
}

const dodo::shader::uniform_struct_array<dodo::shader::uniform_structs::directional_light>& dodo::shader::shader_program::directional_light_locations() const
{
	return directional_light_locations_;
}

const dodo::shader::uniform_struct_array<dodo::shader::uniform_structs::point_light>& dodo::shader::shader_program::point_light_locations() const
{
	return point_light_locations_;
}

const dodo::shader::uniform_locations& dodo::shader::shader_program::uf_locations() const
{
	return uniform_locations_;
}

void dodo::shader::shader_program::print_debug_info(std::ostream& os) const
{
	os << "=========== SHADER INFO ===========\n";
	os << "Ready: " << (ready_ ? "Yes" : "No") << '\n';
	os << "Shader program handle: " << handle_ << '\n';
	os << "Vertex shader: " << (vertex_shader_->ready() ? "OK" : "ERROR") << '\n';
	if (!vertex_shader_->ready())
	{
		os << vertex_shader_->error_str() << '\n';
	}
	os << "Fragment shader: " << (fragment_shader_->ready() ? "OK" : "ERROR") << '\n';
	if (!fragment_shader_->ready())
	{
		os << fragment_shader_->error_str() << '\n';
	}
	if (geometry_shader_ != nullptr)
	{
		os << "Geometry shader: " << (geometry_shader_->ready() ? "OK" : "ERROR") << '\n';
		if (!geometry_shader_->ready())
		{
			os << geometry_shader_->error_str() << '\n';
		}
	}
	os << "Uniforms:\n";
	for (const auto& [name_hash, info] : uniforms_)
		os << "  " << info.name << " (location: " << info.location << ", size: " << info.size << ")\n";
	os << "Recognized uniforms:\n";
	#define _DODO_PRINT_DEBUG_INFO_REC_UNI(x) if (uniform_locations_.x != -1) os << "  " << #x;
	_DODO_PRINT_DEBUG_INFO_REC_UNI(material_color);
	_DODO_PRINT_DEBUG_INFO_REC_UNI(model_matrix);
	_DODO_PRINT_DEBUG_INFO_REC_UNI(view_proj_matrix);
	_DODO_PRINT_DEBUG_INFO_REC_UNI(camera_position);
	_DODO_PRINT_DEBUG_INFO_REC_UNI(material_coefficients);
	_DODO_PRINT_DEBUG_INFO_REC_UNI(directional_light_direction);
	_DODO_PRINT_DEBUG_INFO_REC_UNI(directional_light_color);
	_DODO_PRINT_DEBUG_INFO_REC_UNI(directional_light_array_size);
	_DODO_PRINT_DEBUG_INFO_REC_UNI(point_light_position);
	_DODO_PRINT_DEBUG_INFO_REC_UNI(point_light_color);
	_DODO_PRINT_DEBUG_INFO_REC_UNI(point_light_attenuation);
	_DODO_PRINT_DEBUG_INFO_REC_UNI(point_light_array_size);
	_DODO_PRINT_DEBUG_INFO_REC_UNI(color_texture);
	_DODO_PRINT_DEBUG_INFO_REC_UNI(specular_map);
	_DODO_PRINT_DEBUG_INFO_REC_UNI(environment_map);
	os << "\n===================================\n";
}

void dodo::shader::shader_program::use() const
{
	if (ready_)
	{
		glUseProgram(handle_);
	}
}

bool dodo::shader::shader_program::compile(const bool compile_children)
{
	if (ready_)
	{
		destroy();
	}
	if (compile_children)
	{
		vertex_shader_->compile();
		fragment_shader_->compile();
		if (geometry_shader_ != nullptr) geometry_shader_->compile();
	}
	handle_ = geometry_shader_ == nullptr
		          ? create_shader_program(*vertex_shader_, *fragment_shader_)
		          : create_shader_program(*vertex_shader_, *fragment_shader_, *geometry_shader_);
	if (handle_ == NULL)
	{
		// compilation failed
		ready_ = false;
		return false;
	}
	ready_ = true;

	use();
	query_uniforms();
	directional_light_locations_.determine_locations(*this, uniform_structs::directional_light::name, uniform_array_lengths::directional_light);
	point_light_locations_.determine_locations(*this, uniform_structs::point_light::name, uniform_array_lengths::point_light);

	return true;
}

void dodo::shader::shader_program::destroy(const bool destroy_children)
{
	ready_ = false;

	if (handle_ == NULL)
	{
		return;
	}

	glDeleteProgram(handle_);
	handle_ = NULL;

	if (destroy_children)
	{
		vertex_shader_->destroy();
		fragment_shader_->destroy();
		if (geometry_shader_ != nullptr) geometry_shader_->destroy();
	}
}

bool dodo::shader::shader_program::recompile(const bool recompile_children)
{
	if (recompile_children)
	{
		vertex_shader_->compile();
		fragment_shader_->compile();
		if (geometry_shader_ != nullptr) geometry_shader_->compile();
	}
	destroy();
	return compile();
}

GLint dodo::shader::shader_program::uniform_location(const GLchar* name) const
{
	const auto it = uniforms_.find(util::constexpr_hash(name));
	return it != uniforms_.end() ? it->second.location : -1;
}

GLint dodo::shader::shader_program::lookup_uniform_location(const GLchar* name) const
{
	return glGetUniformLocation(handle_, name);
}

GLint dodo::shader::shader_program::lookup_uniform_location(const std::string& name) const
{
	return glGetUniformLocation(handle_, name.c_str());
}

void dodo::shader::shader_program::set_uniform(GLint location, GLfloat f) const
{
	use();
	glUniform1f(location, f);
}

void dodo::shader::shader_program::set_uniform(GLint location, GLfloat f1, GLfloat f2) const
{
	use();
	glUniform2f(location, f1, f2);
}

void dodo::shader::shader_program::set_uniform(GLint location, const glm::vec2& vec) const
{
	use();
	glUniform2fv(location, 1, glm::value_ptr(vec));
}

void dodo::shader::shader_program::set_uniform(GLint location, GLfloat f1, GLfloat f2, GLfloat f3) const
{
	use();
	glUniform3f(location, f1, f2, f3);
}

void dodo::shader::shader_program::set_uniform(GLint location, const glm::vec3& vec) const
{
	use();
	glUniform3fv(location, 1, glm::value_ptr(vec));
}

void dodo::shader::shader_program::set_uniform(GLint location, GLfloat f1, GLfloat f2, GLfloat f3, GLfloat f4) const
{
	use();
	glUniform4f(location, f1, f2, f3, f4);
}

void dodo::shader::shader_program::set_uniform(GLint location, const glm::vec4& vec) const
{
	use();
	glUniform4fv(location, 1, glm::value_ptr(vec));
}


void dodo::shader::shader_program::set_uniform(GLint location, GLdouble d) const
{
	use();
	glUniform1f(location, d);
}

void dodo::shader::shader_program::set_uniform(GLint location, GLdouble d1, GLdouble d2) const
{
	use();
	glUniform2f(location, d1, d2);
}

void dodo::shader::shader_program::set_uniform(GLint location, const glm::dvec2& vec) const
{
	use();
	glUniform2dv(location, 1, glm::value_ptr(vec));
}

void dodo::shader::shader_program::set_uniform(GLint location, GLdouble d1, GLdouble d2, GLdouble d3) const
{
	use();
	glUniform3d(location, d1, d2, d3);
}

void dodo::shader::shader_program::set_uniform(GLint location, const glm::dvec3& vec) const
{
	use();
	glUniform3dv(location, 1, glm::value_ptr(vec));
}

void dodo::shader::shader_program::set_uniform(GLint location, GLdouble d1, GLdouble d2, GLdouble d3, GLdouble d4) const
{
	use();
	glUniform4d(location, d1, d2, d3, d4);
}

void dodo::shader::shader_program::set_uniform(GLint location, const glm::dvec4& vec) const
{
	use();
	glUniform4dv(location, 1, glm::value_ptr(vec));
}

void dodo::shader::shader_program::set_uniform(GLint location, GLint i) const
{
	use();
	glUniform1i(location, i);
}

void dodo::shader::shader_program::set_uniform(GLint location, GLint i1, GLint i2) const
{
	use();
	glUniform2i(location, i1, i2);
}

void dodo::shader::shader_program::set_uniform(GLint location, const glm::ivec2& vec) const
{
	use();
	glUniform2iv(location, 1, glm::value_ptr(vec));
}

void dodo::shader::shader_program::set_uniform(GLint location, GLint i1, GLint i2, GLint i3) const
{
	use();
	glUniform3i(location, i1, i2, i3);
}

void dodo::shader::shader_program::set_uniform(GLint location, const glm::ivec3& vec) const
{
	use();
	glUniform3iv(location, 1, glm::value_ptr(vec));
}

void dodo::shader::shader_program::set_uniform(GLint location, GLint i1, GLint i2, GLint i3, GLint i4) const
{
	use();
	glUniform4i(location, i1, i2, i3, i4);
}

void dodo::shader::shader_program::set_uniform(GLint location, const glm::ivec4& vec) const
{
	use();
	glUniform4iv(location, 1, glm::value_ptr(vec));
}

void dodo::shader::shader_program::set_uniform(GLint location, GLuint i) const
{
	use();
	glUniform1ui(location, i);
}

void dodo::shader::shader_program::set_uniform(GLint location, GLuint i1, GLuint i2) const
{
	use();
	glUniform2ui(location, i1, i2);
}

void dodo::shader::shader_program::set_uniform(GLint location, GLuint i1, GLuint i2, GLuint i3) const
{
	use();
	glUniform3ui(location, i1, i2, i3);
}

void dodo::shader::shader_program::set_uniform(GLint location, GLuint i1, GLuint i2, GLuint i3, GLuint i4) const
{
	use();
	glUniform4ui(location, i1, i2, i3, i4);
}

void dodo::shader::shader_program::set_uniform(GLint location, const glm::mat3& mat) const
{
	use();
	glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(mat));
}

void dodo::shader::shader_program::set_uniform(GLint location, const glm::mat4& mat) const
{
	use();
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
}

void dodo::shader::shader_program::set_uniform(GLint location, const glm::dmat3& mat) const
{
	use();
	glUniformMatrix3dv(location, 1, GL_FALSE, glm::value_ptr(mat));
}

void dodo::shader::shader_program::set_uniform(GLint location, const glm::dmat4& mat) const
{
	use();
	glUniformMatrix4dv(location, 1, GL_FALSE, glm::value_ptr(mat));
}

void dodo::shader::shader_program::set_uniform(GLint location, const dodo::texture& texture, int tex_unit) const
{
	use();
	texture.bind(location, tex_unit);
}

GLuint dodo::shader::shader_program::create_shader_program(const shader& vertex_shader, const shader& fragment_shader)
{
	if (vertex_shader.ready() && fragment_shader.ready())
	{
		return create_shader_program(vertex_shader.handle(), fragment_shader.handle());
	}
	return NULL;
}

GLuint dodo::shader::shader_program::create_shader_program(const shader& vertex_shader, const shader& fragment_shader, const shader& geometry_shader)
{
	if (vertex_shader.ready() && fragment_shader.ready())
	{
		return create_shader_program(vertex_shader.handle(), fragment_shader.handle(), geometry_shader.handle());
	}
	return NULL;
}

GLuint dodo::shader::shader_program::create_shader_program(const GLuint vertex_shader, const GLuint fragment_shader, const GLuint geometry_shader)
{
	const GLuint shader_program = glCreateProgram();

	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	if (geometry_shader != NULL)
		glAttachShader(shader_program, geometry_shader);

	glLinkProgram(shader_program);

	glValidateProgram(shader_program);
	int validate_status;
	glGetProgramiv(shader_program, GL_VALIDATE_STATUS, &validate_status);
	if (validate_status != GL_TRUE)
	{
		glDeleteProgram(shader_program);
		return NULL;
	}

	return shader_program;
}
