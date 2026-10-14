#pragma once
#include "Shader.h"

namespace dodo::default_shaders
{
	shader::shader_program* basic();
	shader::shader_program* textured_mesh();
	shader::shader_program* skybox();
};
