#include "Renderer.h"
#include <iostream>
#include <memory>
#include "Framebuffer.h"
#include "Light.h"
#include "Shader.h"
#include "glm/ext/vector_float3.hpp"

void dodo::renderer::apply_render_flags()
{
	apply_render_flags((general_flags_ | temp_positive_flags_) & ~temp_negative_flags_);
}

void dodo::renderer::apply_render_flags(render_flag_mask flag_mask)
{
	if (flag_mask & render_flags::points)
	{
		glEnable(GL_PROGRAM_POINT_SIZE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}
	else if (flag_mask & render_flags::wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (flag_mask & render_flags::depth_test)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
	if (flag_mask & render_flags::no_depth_write)
		glDepthMask(GL_FALSE);
	else
		glDepthMask(GL_TRUE);
	if (flag_mask & render_flags::backface_culling)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
	if (flag_mask & render_flags::blend)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendEquation(GL_FUNC_ADD);
	}
	else
	{
		glDisable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ZERO);
		glBlendEquation(GL_FUNC_ADD);
	}
	// todo: kind of a temporary solution. binds the sampler to unit 0, which is used by the renderer for color textures
	if (flag_mask & render_flags::nearest_sampling)
		glBindSampler(0, nearest_texture_sampler);
	else
		glBindSampler(0, 0);
}

dodo::renderer::renderer()
	: utilities_{
		[&] { auto m = dodo::standard_mesh::generate_cube(1,1,1); m.flip_winding_order(); return m; }(), // utilities_.skybox_mesh
		dodo::standard_mesh::generate_2d_rect(2, 2) // utilities_.render_quad
	}
{
	shaders_.text = std::make_unique<shader::shader_program>(
		std::make_shared<shader::shader>(shader::vertex_shader, text_block::shader_source_vertex, shader::source_code),
		std::make_shared<shader::shader>(shader::fragment_shader, text_block::shader_source_fragment,
										 shader::source_code),
		true, true);
	if (!shaders_.text->ready())
	{
		std::cerr << "Failed to compile text shader: " << shaders_.text->error_msg() << std::endl;
		throw std::runtime_error("Failed to compile text shader");
	}
	else
	{
		shaders_.text->print_debug_info(std::cout);
	}

	glGenSamplers(1, &nearest_texture_sampler);
	glSamplerParameteri(nearest_texture_sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glSamplerParameteri(nearest_texture_sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

dodo::renderer::~renderer()
{
	glDeleteSamplers(1, &nearest_texture_sampler);
}

void dodo::renderer::clear(GLbitfield mask) const
{
	glClearColor(clear_color_.r, clear_color_.g, clear_color_.b, clear_color_.a);
	glClear(mask);
}

void dodo::renderer::render(const shader::shader_program& shader)
{
	apply_render_flags();
	shader.use();
	utilities_.render_quad.draw();

	temp_positive_flags_ = 0;
	temp_negative_flags_ = 0;
}

void dodo::renderer::render_model(const model& model, const shader::shader_program& shader, bool manual_mode, GLenum index_layout)
{
	const render_flag_mask flags = (general_flags_ | temp_positive_flags_) & ~temp_negative_flags_ ;

	// activate shader
	shader.use();

	if (!manual_mode)
	{
		// set render modes
		apply_render_flags(flags);
	}

	// model matrix
	glUniformMatrix4fv(shader.uf_locations().model_matrix, 1, false, value_ptr(model.model_transform_matrix_));


	// material coefficients
	if (auto* mat = std::get_if<materials::standard>(&model.material_))
	{
		if(shader.uf_locations().material_color != -1)
			glUniform3f(shader.uf_locations().material_color, mat->color.r, mat->color.g, mat->color.b);

		if (shader.uf_locations().material_coefficients != -1)
			glUniform4f(
				shader.uf_locations().material_coefficients,
				mat->ambient, mat->diffuse, mat->specular, mat->specular_alpha
			);

		if (mat->texture != nullptr && shader.uf_locations().color_texture != -1)
			mat->texture->bind(shader.uf_locations().color_texture, 0);
		if (mat->specular_map != nullptr && shader.uf_locations().specular_map != -1)
			mat->specular_map->bind(shader.uf_locations().specular_map, 1);
		if (mat->environment_map != nullptr && shader.uf_locations().environment_map != -1)
			mat->environment_map->bind(shader.uf_locations().environment_map, 2);

	}
	else if (auto* mat = std::get_if<materials::pbr>(&model.material_))
	{

	}


	// camera
	if (!camera_.expired())
	{
		// camera position
		const std::shared_ptr<dodo::camera> cam(camera_);
		if(shader.uf_locations().camera_position != -1)
			glUniform3f(shader.uf_locations().camera_position, cam->position().x, cam->position().y, cam->position().z);

		// view-projection matrix
		if (shader.uf_locations().view_proj_matrix != -1)
			glUniformMatrix4fv(shader.uf_locations().view_proj_matrix, 1, false, glm::value_ptr(cam->camera_matrix()));
	}
	else
	{
		// TODO: should probably remove this to let users set camera_position and view-proj_matrix themselves.
		// if (shader.uf_locations().camera_position != -1)
		// 	glUniform3f(shader.uf_locations().camera_position, 0, 0, 0);
		// if (shader.uf_locations().view_proj_matrix != -1)
		// 	glUniformMatrix4fv(shader.uf_locations().view_proj_matrix, 1, false, value_ptr(glm::mat4(1)));
	}


	// iterate over directional lights
	if (shader.uf_locations().directional_light_array_size != -1)
	{
		const auto& directional_light_locations = shader.directional_light_locations();
		unsigned i = 0;
		for (auto& dl : directional_light_)
		{
			if (!dl.expired())
			{
				const std::shared_ptr<dodo::directional_light> directional_light(dl);
				// directional light direction
				const auto& direction = glm::normalize(directional_light->direction());
				glUniform3f(directional_light_locations[i].direction, direction.x, direction.y, direction.z);
				// directional light color
				const auto& color = directional_light->color();
				glUniform3f(directional_light_locations[i].color, color.r, color.g, color.b);
				i++;
				if (i == directional_light_locations.size())
				{
					break;
				}
			}
		}
		// amount of directional lights
		glUniform1ui(shader.uf_locations().directional_light_array_size, i);
	}
	// no directional light array present, attempt single light mode
	else if (!directional_light_.empty() && !directional_light_.begin()->expired())
	{
		const std::shared_ptr<dodo::directional_light> directional_light(*directional_light_.begin());
		// directional light direction
		if (shader.uf_locations().directional_light_direction != -1)
		{
			const auto& direction = glm::normalize(directional_light->direction());
			glUniform3f(shader.uf_locations().directional_light_direction, direction.x, direction.y, direction.z);
		}
		// directional light color
		if (shader.uf_locations().directional_light_color != -1)
		{
			const auto& color = directional_light->color();
			glUniform3f(shader.uf_locations().directional_light_color, color.r, color.g, color.b);
		}
	}

	// iterate over point lights
	if (shader.uf_locations().point_light_array_size != -1)
	{
		const auto& point_light_locations = shader.point_light_locations();
		unsigned i = 0;
		for (auto& dl : point_light_)
		{
			if (!dl.expired())
			{
				const std::shared_ptr<dodo::point_light> point_light(dl);
				// point light position
				const auto& position = point_light->position();
				glUniform3f(point_light_locations[i].position, position.x, position.y, position.z);
				// point light color
				const auto& color = point_light->color();
				glUniform3f(point_light_locations[i].color, color.r, color.g, color.b);
				// point light attenuation
				const auto& attenuation = point_light->attenuation();
				glUniform3f(point_light_locations[i].attenuation, attenuation.x, attenuation.y, attenuation.z);
				i++;
				if (i == point_light_locations.size())
				{
					break;
				}
			}
		}
		// amount of point lights
		glUniform1ui(shader.uf_locations().point_light_array_size, i);
	}
	// no point light array present, attempt single light mode
	else if (!point_light_.empty() && !point_light_.begin()->expired())
	{
		const std::shared_ptr<dodo::point_light> point_light(*point_light_.begin());
		// point light position
		if (shader.uf_locations().point_light_position != -1)
		{
			const auto& position = point_light->position();
			glUniform3f(shader.uf_locations().point_light_position, position.x, position.y, position.z);
		}
		// point light color
		if (shader.uf_locations().point_light_color != -1)
		{
			const auto& color = point_light->color();
			glUniform3f(shader.uf_locations().point_light_color, color.r, color.g, color.b);
		}
		// point light attenuation
		if (shader.uf_locations().point_light_attenuation != -1)
		{
			const auto& attenuation = point_light->attenuation();
			glUniform3f(shader.uf_locations().point_light_attenuation, attenuation.x, attenuation.y, attenuation.z);
		}
	}

	if (flags & render_flags::points)
	{
		model.draw_points();
	}
	else
	{
		model.draw(index_layout);
	}

	if (!manual_mode)
	{
		glDisable(GL_PROGRAM_POINT_SIZE);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_BLEND);
	}

	temp_positive_flags_ = 0;
	temp_negative_flags_ = 0;
}


void dodo::renderer::render_skybox(const texture& cube_map, const shader::shader_program& shader)
{
	const render_flag_mask flags = (general_flags_ | temp_positive_flags_) & ~temp_negative_flags_;

	if (shader.uf_locations().environment_map == -1 || shader.uf_locations().view_proj_matrix == -1)
		return;

	if (flags & render_flags::wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	shader.use();

	// environment map
	cube_map.bind(shader.uf_locations().environment_map, 0);


	// transformation matrix
	if (!camera_.expired())
	{
		const std::shared_ptr<dodo::camera> cam(camera_);

		glUniformMatrix4fv(
			shader.uf_locations().view_proj_matrix, 1, false,
			glm::value_ptr(glm::translate(cam->camera_matrix(), cam->position()))
		);
	}


	utilities_.skybox_mesh.draw();

	glDepthFunc(GL_LESS);

	temp_positive_flags_ = 0;
	temp_negative_flags_ = 0;
}

void dodo::renderer::render_texture(const texture& texture, const shader::shader_program& shader)
{
	const render_flag_mask flags = (general_flags_ | temp_positive_flags_) & ~temp_negative_flags_;

	glDisable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	shader.use();
	texture.bind(shader.uf_locations().color_texture, 0);
	utilities_.render_quad.draw();

	glEnable(GL_DEPTH_TEST);

	temp_positive_flags_ = 0;
	temp_negative_flags_ = 0;
}

void dodo::renderer::render_text(text_block &text, glm::vec2 pos, float scale, glm::vec3 color)
{
	shaders_.text->set_uniform<"atlas_glyphs_per_row", int>(text.font_->glyphs_per_row());
	shaders_.text->set_uniform<"atlas_glyph_scale", glm::vec2>(text.font_->glyph_uv_scale());
	shaders_.text->set_uniform<"grid_size", glm::ivec2>({text.columns_, text.rows_});
	shaders_.text->set_uniform<"cell_size", glm::vec2>(
		{float(font::glyph::width) / framebuffer::get_bound_framebuffer()->width(),
		 float(font::glyph::height) / framebuffer::get_bound_framebuffer()->height()});
	shaders_.text->set_uniform<"atlas">(text.font_->atlas(), 0);
	shaders_.text->set_uniform<"color">(color);
	shaders_.text->set_uniform<"scale">(scale);
	shaders_.text->set_uniform<"origin">(pos);

	glDisable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);
	glBindSampler(0, nearest_texture_sampler);
	shaders_.text->use();
	text.update();
	text.mesh_.draw();
	glEnable(GL_DEPTH_TEST);

	temp_positive_flags_ = 0;
	temp_negative_flags_ = 0;
}

void dodo::renderer::prepare_state(render_flag_mask positive_flags, render_flag_mask negative_flags)
{
	apply_render_flags((general_flags_ | positive_flags) & ~negative_flags);
}

void dodo::renderer::add_directional_light(const std::weak_ptr<directional_light>& directional_light)
{
	directional_light_.push_back(directional_light);
}

void dodo::renderer::add_point_light(const std::weak_ptr<point_light>& point_light)
{
	point_light_.push_back(point_light);
}

void dodo::renderer::set_clear_color(const glm::vec4& clear_color)
{
	clear_color_ = clear_color;
}

void dodo::renderer::set_clear_color(const glm::vec3& clear_color)
{
	clear_color_ = glm::vec4(clear_color, 1.0f);
}

void dodo::renderer::set_camera(const std::shared_ptr<dodo::camera>& camera)
{
	camera_ = camera;
}

void dodo::renderer::set_backface_culling(bool b)
{
	//render_flags_.backface_culling = b;
}

void dodo::renderer::set_depth_test(bool b)
{
	//render_flags_.depth_test = b;
}

void dodo::renderer::set_wireframe(bool b)
{
	//render_flags_.wireframe = b;
}

void dodo::renderer::set_point_rendering(bool b)
{
	//render_flags_.points = b;
}

void dodo::renderer::set_blend(bool b)
{
	//render_flags_.blend = b;
}

void dodo::renderer::set_flags(render_flag_mask flags, render_flag_mask mask)
{
	general_flags_ = (general_flags_ & ~mask) | (flags & mask);
}

void dodo::renderer::enable_flags(render_flag_mask positive_flags)
{
	general_flags_ |= positive_flags;
}

void dodo::renderer::disable_flags(render_flag_mask negative_flags)
{
	general_flags_ &= ~negative_flags;
}

dodo::renderer::render_flag_mask dodo::renderer::flags() const
{
	return general_flags_;
}

void dodo::renderer::next_call(render_flag_mask positive_flags, render_flag_mask negative_flags)
{
	temp_positive_flags_ = positive_flags;
	temp_negative_flags_ = negative_flags;
}

glm::vec4 dodo::renderer::clear_color() const
{
	return clear_color_;
}

std::weak_ptr<dodo::camera> dodo::renderer::camera() const
{
	return camera_;
}

bool dodo::renderer::backface_culling() const
{
	return general_flags_ & render_flags::backface_culling;
}

bool dodo::renderer::depth_test() const
{
	return general_flags_ & render_flags::depth_test;
}

bool dodo::renderer::wireframe() const
{
	return general_flags_ & render_flags::wireframe;
}
