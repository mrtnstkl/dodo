#include "Mesh.h"

#include "Shader.h"


void dodo::vertex_structs::vertex3_norm_uv::configure_vao(GLuint vao, GLuint vbo, GLuint vib)
{
	// bind VAO
	glBindVertexArray(vao);

	// bind VBO
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// enable attributes
	glEnableVertexAttribArray(shader::input_locations::vertex_position);
	glVertexAttribPointer(shader::input_locations::vertex_position, 3, GL_FLOAT, GL_FALSE, sizeof(vertex3_norm_uv), (void*)(0));

	glEnableVertexAttribArray(shader::input_locations::vertex_normal);
	glVertexAttribPointer(shader::input_locations::vertex_normal, 3, GL_FLOAT, GL_FALSE, sizeof(vertex3_norm_uv), (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(shader::input_locations::vertex_texture_coordinate);
	glVertexAttribPointer(shader::input_locations::vertex_texture_coordinate, 2, GL_FLOAT, GL_FALSE, sizeof(vertex3_norm_uv), (void*)(6 * sizeof(float)));


	// bind index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vib);

	// unbind VAO, then VBO
	glBindVertexArray(NULL);
	glBindBuffer(GL_ARRAY_BUFFER, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);
}

void dodo::vertex_structs::vertex3::configure_vao(GLuint vao, GLuint vbo, GLuint vib)
{
	// bind VAO
	glBindVertexArray(vao);

	// bind VBO
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// enable attributes
	glEnableVertexAttribArray(shader::input_locations::vertex_position);
	glVertexAttribPointer(shader::input_locations::vertex_position, 3, GL_FLOAT, GL_FALSE, sizeof(vertex3), (void*)(0));

	// bind index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vib);

	// unbind VAO, then VBO
	glBindVertexArray(NULL);
	glBindBuffer(GL_ARRAY_BUFFER, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);
}


void dodo::mesh_base::create_buffers()
{
	// create VAO, VBO and VIB
	glGenVertexArrays(1, &vao_handle_);
	glGenBuffers(1, &vbo_handle_);
	glGenBuffers(1, &vib_handle_);
}

void dodo::standard_mesh::generate_normals()
{

	for (int i = 0; i + 2 < indices_.size(); i += 3)
	{
		const uint32_t v1 = indices_[i];
		const uint32_t v2 = indices_[i + 1];
		const uint32_t v3 = indices_[i + 2];

		const glm::vec3 a = glm::vec3{ vertices_[v2].x,vertices_[v2].y, vertices_[v2].z } - glm::vec3{ vertices_[v1].x, vertices_[v1].y, vertices_[v1].z };
		const glm::vec3 b = glm::vec3{ vertices_[v3].x,vertices_[v3].y, vertices_[v3].z } - glm::vec3{ vertices_[v1].x, vertices_[v1].y, vertices_[v1].z };
		const glm::vec3 triangle_normal = glm::normalize(glm::vec3{
			(a.y * b.z) - (a.z * b.y),
			(a.z * b.x) - (a.x * b.z),
			(a.x * b.y) - (a.y * b.x)
			});

		glm::vec3 v1_normal = { vertices_[v1].norm_x, vertices_[v1].norm_y, vertices_[v1].norm_z };
		glm::vec3 v2_normal = { vertices_[v2].norm_x, vertices_[v2].norm_y, vertices_[v2].norm_z };
		glm::vec3 v3_normal = { vertices_[v3].norm_x, vertices_[v3].norm_y, vertices_[v3].norm_z };

		v1_normal = glm::normalize(v1_normal + triangle_normal);
		v2_normal = glm::normalize(v2_normal + triangle_normal);
		v3_normal = glm::normalize(v3_normal + triangle_normal);

		vertices_[v1].norm_x = v1_normal.x;
		vertices_[v1].norm_y = v1_normal.y;
		vertices_[v1].norm_z = v1_normal.z;

		vertices_[v2].norm_x = v2_normal.x;
		vertices_[v2].norm_y = v2_normal.y;
		vertices_[v2].norm_z = v2_normal.z;

		vertices_[v3].norm_x = v3_normal.x;
		vertices_[v3].norm_y = v3_normal.y;
		vertices_[v3].norm_z = v3_normal.z;
	}
}


dodo::mesh_base::mesh_base()
{
	create_buffers();
}

dodo::mesh_base::~mesh_base()
{
	glDeleteVertexArrays(1, &vao_handle_);
	glDeleteBuffers(1, &vbo_handle_);
	glDeleteBuffers(1, &vib_handle_);
}

dodo::mesh_base::mesh_base(const dodo::mesh_base& other)
	: vertex_count_(other.vertex_count_),
	index_count_(other.index_count_)
{
	create_buffers();
	// todo: this is still untested, verify that it works
	// get size of other buffers
	// vertex buffer
	GLint vertex_buf_size;
	glBindBuffer(GL_ARRAY_BUFFER, other.vbo_handle_);
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &vertex_buf_size);
	// index buffer
	GLint index_buf_size;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, other.vib_handle_);
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &index_buf_size);
	// initialize buffers
	// vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo_handle_);
	glBufferData(GL_ARRAY_BUFFER, vertex_buf_size, NULL, GL_STATIC_DRAW);
	// index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vib_handle_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buf_size, NULL, GL_STATIC_DRAW);
	// copy buffer data
	// vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, other.vbo_handle_);
	glBindBuffer(GL_COPY_WRITE_BUFFER, vbo_handle_);
	glCopyBufferSubData(GL_ARRAY_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, vertex_buf_size);
	// index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, other.vib_handle_);
	glBindBuffer(GL_COPY_WRITE_BUFFER, vib_handle_);
	glCopyBufferSubData(GL_ELEMENT_ARRAY_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, index_buf_size);
	// unbind buffers
	glBindBuffer(GL_ARRAY_BUFFER, NULL);
	glBindBuffer(GL_COPY_WRITE_BUFFER, NULL);
}

dodo::mesh_base::mesh_base(mesh_base&& other) noexcept
	: vertex_count_(other.vertex_count_),
	index_count_(other.index_count_)
{
	vao_handle_ = other.vao_handle_;
	vbo_handle_ = other.vbo_handle_;
	vib_handle_ = other.vib_handle_;

	other.vao_handle_ = NULL;
	other.vbo_handle_ = NULL;
	other.vib_handle_ = NULL;
}

dodo::mesh_base& dodo::mesh_base::operator=(const mesh_base& other)
{
	if (this == &other)
	{
		return *this;
	}
	vertex_count_ = other.vertex_count_;
	index_count_ = other.index_count_;

	// todo: this is still untested, verify that it works
	// delete buffers
	glDeleteBuffers(1, &vbo_handle_);
	glDeleteBuffers(1, &vib_handle_);
	// get size of other buffers
	// vertex buffer
	GLint vertex_buf_size;
	glBindBuffer(GL_ARRAY_BUFFER, other.vbo_handle_);
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &vertex_buf_size);
	// index buffer
	GLint index_buf_size;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, other.vib_handle_);
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &index_buf_size);
	// initialize buffers
	// vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo_handle_);
	glBufferData(GL_ARRAY_BUFFER, vertex_buf_size, NULL, GL_STATIC_DRAW);
	// index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vib_handle_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buf_size, NULL, GL_STATIC_DRAW);
	// copy buffer data
	// vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, other.vbo_handle_);
	glBindBuffer(GL_COPY_WRITE_BUFFER, vbo_handle_);
	glCopyBufferSubData(GL_ARRAY_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, vertex_buf_size);
	// index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, other.vib_handle_);
	glBindBuffer(GL_COPY_WRITE_BUFFER, vib_handle_);
	glCopyBufferSubData(GL_ELEMENT_ARRAY_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, index_buf_size);
	// unbind buffers
	glBindBuffer(GL_ARRAY_BUFFER, NULL);
	glBindBuffer(GL_COPY_WRITE_BUFFER, NULL);

	return *this;
}

dodo::mesh_base& dodo::mesh_base::operator=(mesh_base&& other) noexcept
{
	vertex_count_ = other.vertex_count_;
	index_count_ = other.index_count_;

	vao_handle_ = other.vao_handle_;
	vbo_handle_ = other.vbo_handle_;
	vib_handle_ = other.vib_handle_;

	other.vao_handle_ = NULL;
	other.vbo_handle_ = NULL;
	other.vib_handle_ = NULL;

	return *this;
}


void dodo::mesh_base::draw(GLenum mode) const
{
	glBindVertexArray(vao_handle_);
	glDrawElements(mode, index_count_, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(NULL);
}

void dodo::mesh_base::draw_triangles() const
{
	// todo: implement
	assert(false);
}

void dodo::mesh_base::draw_points() const
{
	glBindVertexArray(vao_handle_);
	glDrawArrays(GL_POINTS, 0, vertex_count_);
	glBindVertexArray(NULL);
}

void dodo::mesh_base::draw_instanced(int amount) const
{
	glBindVertexArray(vao_handle_);
	glDrawElementsInstanced(GL_TRIANGLES, index_count_, GL_UNSIGNED_INT, nullptr, amount);
	glBindVertexArray(NULL);
}

GLuint dodo::mesh_base::vao_handle() const
{
	return vao_handle_;
}

GLuint dodo::mesh_base::vbo_handle() const
{
	return vbo_handle_;
}

GLuint dodo::mesh_base::vib_handle() const
{
	return vib_handle_;
}

dodo::standard_mesh dodo::standard_mesh::generate_cube(float width, float height, float depth)
{
	width /= 2.0f;
	height /= 2.0f;
	depth /= 2.0f;

	dodo::standard_mesh mesh;


	// bottom plane
	const uint32_t bottom_1 = mesh.add_vertex({-width, -height, -depth, 0, 0, 0, 0, 0});
	const uint32_t bottom_2 = mesh.add_vertex({width, -height, -depth, 0, 0, 0, 1, 0});
	const uint32_t bottom_3 = mesh.add_vertex({width, -height, depth, 0, 0, 0, 1, 1});
	const uint32_t bottom_4 = mesh.add_vertex({-width, -height, depth, 0, 0, 0, 0, 1});
	mesh.add_triangle(bottom_1, bottom_2, bottom_3);
	mesh.add_triangle(bottom_4, bottom_1, bottom_3);

	// top plane
	const uint32_t top_1 = mesh.add_vertex({width, height, -depth, 0, 0, 0, 1, 1});
	const uint32_t top_2 = mesh.add_vertex({-width, height, -depth, 0, 0, 0, 0, 1});
	const uint32_t top_3 = mesh.add_vertex({width, height, depth, 0, 0, 0, 1, 0});
	const uint32_t top_4 = mesh.add_vertex({-width, height, depth, 0, 0, 0, 0, 0});
	mesh.add_triangle(top_1, top_2, top_3);
	mesh.add_triangle(top_2, top_4, top_3);

	// back plane
	const uint32_t back_1 = mesh.add_vertex({width, -height, -depth, 0, 0, 0, 1, 1});
	const uint32_t back_2 = mesh.add_vertex({-width, -height, -depth, 0, 0, 0, 0, 1});
	const uint32_t back_3 = mesh.add_vertex({-width, height, -depth, 0, 0, 0, 0, 0});
	const uint32_t back_4 = mesh.add_vertex({width, height, -depth, 0, 0, 0, 1, 0});
	mesh.add_triangle(back_1, back_2, back_3);
	mesh.add_triangle(back_4, back_1, back_3);

	// front plane
	const uint32_t front_1 = mesh.add_vertex({-width, -height, depth, 0, 0, 0, 0, 0});
	const uint32_t front_2 = mesh.add_vertex({width, -height, depth, 0, 0, 0, 1, 0});
	const uint32_t front_3 = mesh.add_vertex({width, height, depth, 0, 0, 0, 1, 1});
	const uint32_t front_4 = mesh.add_vertex({ -width, height, depth, 0, 0, 0, 0, 1 });
	mesh.add_triangle(front_1, front_2, front_3);
	mesh.add_triangle(front_4, front_1, front_3);

	// left plane
	const uint32_t left_1 = mesh.add_vertex({-width, height, -depth, 0, 0, 0, 0, 1});
	const uint32_t left_2 = mesh.add_vertex({-width, -height, -depth, 0, 0, 0, 0, 0});
	const uint32_t left_3 = mesh.add_vertex({-width, height, depth, 0, 0, 0, 1, 1});
	const uint32_t left_4 = mesh.add_vertex({ -width, -height, depth, 0, 0, 0, 1, 0 });
	mesh.add_triangle(left_1, left_2, left_3);
	mesh.add_triangle(left_2, left_4, left_3);

	// right plane
	const uint32_t right_1 = mesh.add_vertex({width, height, depth, 0, 0, 0, 0, 1});
	const uint32_t right_2 = mesh.add_vertex({width, -height, depth, 0, 0, 0, 0, 0});
	const uint32_t right_3 = mesh.add_vertex({width, height, -depth, 0, 0, 0, 1, 1});
	const uint32_t right_4 = mesh.add_vertex({ width, -height, -depth, 0, 0, 0, 1, 0 });
	mesh.add_triangle(right_1, right_2, right_3);
	mesh.add_triangle(right_2, right_4, right_3);

	mesh.generate_normals();
	mesh.write_buffers();
	return mesh;
}

dodo::standard_mesh dodo::standard_mesh::generate_cylinder(float radius, float height, uint32_t segments)
{
	segments = segments < 3 ? 3 : segments;

	dodo::standard_mesh mesh;

	const uint32_t top_center = mesh.add_vertex({
		0, height / 2, 0,
		0, 0, 0,
		0.5, 0.5
	});
	const uint32_t bottom_center = mesh.add_vertex({
		0, -height / 2, 0,
		0, 0, 0,
		0.5, 0.5
	});

	uint32_t* top_ring_1 = new uint32_t[segments];
	uint32_t* bottom_ring_1 = new uint32_t[segments];
	uint32_t* top_ring_2 = new uint32_t[segments + 1];
	uint32_t* bottom_ring_2 = new uint32_t[segments + 1];

	const float step_size = glm::pi<float>() * 2.0f / segments;

	for (int i = 0; i < segments; ++i)
	{
		top_ring_1[i] = mesh.add_vertex({
			cosf(step_size * i) * radius, height / 2, sinf(step_size * i) * radius,
			0, 0, 0,
			(cosf(step_size * i) + 1) / 2, (sinf(step_size * i) + 1) / 2
		});
		bottom_ring_1[i] = mesh.add_vertex({
			cosf(step_size * i) * radius, -height / 2, sinf(step_size * i) * radius,
			0, 0, 0,
			(cosf(step_size * i) + 1) / 2, (sinf(step_size * i) + 1) / 2
		});

		top_ring_2[i] = mesh.add_vertex({
			cosf(step_size * i) * radius, height / 2, sinf(step_size * i) * radius,
			0, 0, 0,
			static_cast<float>(i) / static_cast<float>(segments), 1
		});
		bottom_ring_2[i] = mesh.add_vertex({
			cosf(step_size * i) * radius, -height / 2, sinf(step_size * i) * radius,
			0, 0, 0,
			static_cast<float>(i) / static_cast<float>(segments), 0
		});
	}

	// additional vertices for texture seam
	top_ring_2[segments] = mesh.add_vertex({radius, height / 2, 0, 0, 0, 0, 1, 1});
	bottom_ring_2[segments] = mesh.add_vertex({radius, height / -2, 0, 0, 0, 0, 1, 0});

	for (int i = 0; i < segments; ++i)
	{
		// top and bottom caps
		mesh.add_triangle(top_ring_1[i], top_ring_1[i == 0 ? segments - 1 : i - 1], top_center);
		mesh.add_triangle(bottom_ring_1[i], bottom_ring_1[(i + 1) % segments], bottom_center);

		// walls
		mesh.add_triangle(bottom_ring_2[i], top_ring_2[i], top_ring_2[(i + 1)]);
		mesh.add_triangle(bottom_ring_2[(i + 1)], bottom_ring_2[i], top_ring_2[(i + 1)]);
	}

	mesh.generate_normals();

	// fix normals of seam vertices
	mesh.vertices_[top_ring_2[0]] = {radius, height / 2, 0, 1, 0, 0, 0, 1};
	mesh.vertices_[bottom_ring_2[0]] = {radius, -height / 2, 0, 1, 0, 0, 0, 0};
	mesh.vertices_[top_ring_2[segments]] = {radius, height / 2, 0, 1, 0, 0, 1, 1};
	mesh.vertices_[bottom_ring_2[segments]] = {radius, -height / 2, 0, 1, 0, 0, 1, 0};

	delete[] top_ring_1;
	delete[] bottom_ring_1;
	delete[] top_ring_2;
	delete[] bottom_ring_2;

	mesh.write_buffers();
	return mesh;
}

dodo::standard_mesh dodo::standard_mesh::generate_sphere(float radius, uint32_t longitudinal_segments, uint32_t latitudinal_segments)
{
	longitudinal_segments = longitudinal_segments < 3 ? 3 : longitudinal_segments;
	latitudinal_segments = latitudinal_segments < 2 ? 2 : latitudinal_segments;

	dodo::standard_mesh mesh;

	uint32_t** vertices = new uint32_t * [latitudinal_segments - 1];
	for (int i = 0; i < latitudinal_segments - 1; ++i)
	{
		vertices[i] = new uint32_t[longitudinal_segments + 1];
	}


	const float polar_step_size = glm::pi<float>() * 1.0f / static_cast<float>(latitudinal_segments);
	const float azi_step_size = glm::pi<float>() * 2.0f / static_cast<float>(longitudinal_segments);


	for (int polar_index = 0; polar_index < latitudinal_segments - 1; ++polar_index)
	{
		for (int azi_index = 0; azi_index < longitudinal_segments + 1; ++azi_index)
		{
			const float polar = static_cast<float>(polar_index) + 1.0f;
			const float azi = static_cast<float>(azi_index);

			vertices[polar_index][azi_index] = mesh.add_vertex({
				radius * sinf(polar_step_size * polar) * cosf(azi_step_size * azi),
				radius * cosf(polar_step_size * polar),
				radius * sinf(polar_step_size * polar) * sinf(azi_step_size * azi),
				0, 0, 0,
				static_cast<float>(azi_index) / static_cast<float>(longitudinal_segments),
				static_cast<float>(polar_index + 1) / static_cast<float>(latitudinal_segments)
			});
		}
	}

	for (int polar = 0; polar < latitudinal_segments - 2; ++polar)
	{
		for (int azi = 0; azi < longitudinal_segments; ++azi)
		{
			mesh.add_triangle(vertices[polar][azi + 1], vertices[polar + 1][azi + 1], vertices[polar][azi]);
			mesh.add_triangle(vertices[polar + 1][azi + 1], vertices[polar + 1][azi], vertices[polar][azi]);
		}
	}

	uint32_t* top_cap = new uint32_t[longitudinal_segments];
	uint32_t* bottom_cap = new uint32_t[longitudinal_segments];

	// caps
	for (int azi = 0; azi < longitudinal_segments; ++azi)
	{
		top_cap[azi] = mesh.add_vertex({0, radius, 0, 0, 0, 0, 1.0f / longitudinal_segments * azi, 0});
		bottom_cap[azi] = mesh.add_vertex({0, -radius, 0, 0, 0, 0, 1.0f / longitudinal_segments * azi, 1});

		mesh.add_triangle(top_cap[azi], vertices[0][azi + 1], vertices[0][azi]);
		mesh.add_triangle(bottom_cap[azi], vertices[latitudinal_segments - 2][azi], vertices[latitudinal_segments - 2][azi + 1]);
	}

	mesh.generate_normals();

	// fix seam normals
	for (int i = 0; i < latitudinal_segments - 1; ++i)
	{
		// todo: not optimal solution, seam is noticeable in reflections
		auto& v1 = mesh.vertices_[vertices[i][0]];
		auto& v2 = mesh.vertices_[vertices[i][longitudinal_segments]];
		const float len = sqrt(v1.norm_x * v1.norm_x + v1.norm_y * v1.norm_y);
		v1.norm_x /= len; v1.norm_y /= len; v1.norm_z = 0;
		v2.norm_x = v1.norm_x; v2.norm_y = v1.norm_y; v2.norm_z = v1.norm_z;
	}

	// fix cap normals
	for (int i = 0; i < longitudinal_segments; ++i)
	{
		// todo: not optimal solution, cap is not smooth
		auto& v1 = mesh.vertices_[top_cap[i]];
		auto& v2 = mesh.vertices_[bottom_cap[i]];
		v1.norm_x = 0; v1.norm_y = 1; v1.norm_z = 0;
		v2.norm_x = 0; v2.norm_y = -1; v2.norm_z = 0;
	}


	for (int i = 0; i < latitudinal_segments - 1; ++i)
	{
		delete[] vertices[i];
	}
	delete[] vertices;
	delete[] top_cap;
	delete[] bottom_cap;

	mesh.write_buffers();
	return mesh;
}


dodo::standard_mesh dodo::standard_mesh::generate_2d_rect(float height, float width)
{
	dodo::standard_mesh mesh;

	height *= .5f;
	width *= .5f;

	const auto lower_left = mesh.add_vertex({-width, -height, 0, 0, 0, 0, 0, 0});
	const auto upper_right = mesh.add_vertex({width, height, 0, 0, 0, 0, 1, 1});

	mesh.add_triangle(lower_left, upper_right, mesh.add_vertex({-width, height, 0, 0, 0, 0, 0, 1}));
	mesh.add_triangle(upper_right, lower_left, mesh.add_vertex({width, -height, 0, 0, 0, 0, 1, 0}));

	mesh.generate_normals();
	mesh.write_buffers();
	return mesh;
}


dodo::standard_mesh dodo::standard_mesh::from_obj_file(const char* path, const bool generate_normals)
{
	dodo::standard_mesh mesh;
	std::ifstream in(path);

	enum
	{
		V = 0,	// vertex position
		VT,		// texture coordinate
		VN,		// vertex normal
		F,		// face
		L,		// line
		NOTHING
	}
	line_type = NOTHING;

	int counter[5] = { 0, 0, 0, 0, 0 };


	while (true)
	{
		if (!in.good())
			break;
		std::string line;
		std::getline(in, line);
		std::istringstream line_stream(line);


		if (line.size() < 4)
			continue;

		if (line[0] == 'v' && line[1] == ' ')
			line_type = V;
		else if (line[0] == 'v' && line[1] == 't' && line[2] == ' ')
			line_type = VT;
		else if (line[0] == 'v' && line[1] == 'n' && line[2] == ' ')
			line_type = VN;
		else if (line[0] == 'f' && line[1] == ' ')
			line_type = F;
		else if (line[0] == 'l' && line[1] == ' ')
			line_type = L;


		switch (line_type)
		{
		case V:
		{
			if (mesh.vertices_.size() <= counter[V])
			{
				mesh.vertices_.push_back({});
			}

			char temp[2];
			line_stream >> temp >> mesh.vertices_[counter[V]].x >> mesh.vertices_[counter[V]].y >> mesh.vertices_[counter[V]].z;
			counter[V]++;
		}
		break;

		case VT:
			break;

		case VN:
		{
			if (mesh.vertices_.size() <= counter[VN])
			{
				mesh.vertices_.push_back({});
			}

			char temp[3];
			glm::vec3 normal = { 0, 0, 0 };
			line_stream >> temp >> normal.x >> normal.y >> normal.z;

			normal = glm::normalize(normal);
			mesh.vertices_[counter[VN]].norm_x = normal.x;
			mesh.vertices_[counter[VN]].norm_y = normal.y;
			mesh.vertices_[counter[VN]].norm_z = normal.z;
			counter[VN]++;
		}
		break;

		case F:
		{
			uint32_t v[3] = { 0, 0, 0 };
			uint32_t vt[3] = { 0, 0, 0 }; // unused
			uint32_t vn[3] = { 0, 0, 0 }; // unused

			char temp;
			line_stream >> temp;

			for (int i = 0; i < 3 && line_stream.good(); i++)
			{
				char vertex[64];

				// get next vertex. a vertex has one of the following formats: "1", "1/2", "1/2/3", "1//3"
				line_stream >> vertex;

				// count '/' characters
				int delimiter_count = 0;
				const int vertex_strlen = strlen(vertex);
				for (int j = 0; j < vertex_strlen; j++)
				{
					if (vertex[j] == '/')
					{
						delimiter_count++;
					}
				}

				const char* delimiter = "/";
				char* strtok_context;

#if defined(_WIN32) || defined(_WIN64)
				char* token1 = strtok_s(vertex, delimiter, &strtok_context);
				char* token2 = strtok_s(nullptr, delimiter, &strtok_context);
				char* token3 = strtok_s(nullptr, delimiter, &strtok_context);
#else
				char* token1 = strtok_r(vertex, delimiter, &strtok_context);
				char* token2 = strtok_r(nullptr, delimiter, &strtok_context);
				char* token3 = strtok_r(nullptr, delimiter, &strtok_context);
#endif

				v[i] = std::atoi(token1);

				if (delimiter_count == 1) // "1/2"
				{
					vt[i] = std::atoi(token2);
				}
				else if (delimiter_count == 2 && token3 != nullptr)
				{
					if (token3 == nullptr) // "1//3"
					{
						vt[i] = NULL;
						vn[i] = std::atoi(token2);

					}
					else // "1/2/3
					{
						vt[i] = std::atoi(token2);
						vn[i] = std::atoi(token3);
					}
				}
			}

			if (mesh.indices_.size() <= counter[F] + 2)
			{
				mesh.indices_.resize(mesh.indices_.size() + 3);
			}

			mesh.add_triangle(v[0] - 1, v[1] - 1, v[2] - 1);
		}
		break;

		case L:
			break;

		case NOTHING:

		default:;
		}
	}

	mesh.vertex_count_ = mesh.vertices_.size();
	mesh.index_count_ = mesh.indices_.size();

	in.close();
	if (generate_normals)
	{
		mesh.generate_normals();
	}
	mesh.write_buffers();
	return mesh;
}


void dodo::standard_mesh::write_to_obj(const char *path) const
{
	std::ofstream file(path);
	if (!file.is_open())
	{
		return;
	}

	// Write vertices
	for (const auto &v : vertices_)
	{
		file << "v " << v.x << " " << v.y << " " << v.z << "\n";
		file << "vn " << v.norm_x << " " << v.norm_y << " " << v.norm_z << "\n";
		file << "vt " << v.tex_u << " " << v.tex_v << "\n";
	}

	// Write faces (indices)
	for (size_t i = 0; i < indices_.size(); i += 3)
	{
		file << "f ";
		for (size_t j = 0; j < 3; ++j)
		{
			// OBJ indices are 1-based
			file << indices_[i + j] + 1 << "/";
			file << indices_[i + j] + 1 << "/";
			file << indices_[i + j] + 1 << " ";
		}
		file << "\n";
	}

	file.close();
}
