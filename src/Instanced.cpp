#include "Instanced.h"

dodo::instanced::array_structs::transformation::transformation(glm::mat4 trans_mat)
	:trans_mat(trans_mat)
{
}

void dodo::instanced::array_structs::transformation::bind(GLuint starting_index)
{
	glEnableVertexAttribArray(starting_index + 0);
	glEnableVertexAttribArray(starting_index + 1);
	glEnableVertexAttribArray(starting_index + 2);
	glEnableVertexAttribArray(starting_index + 3);

	glVertexAttribPointer(starting_index + 0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(0));
	glVertexAttribPointer(starting_index + 1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(1 * sizeof(glm::vec4)));
	glVertexAttribPointer(starting_index + 2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
	glVertexAttribPointer(starting_index + 3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

	glVertexAttribDivisor(starting_index + 0, 1);
	glVertexAttribDivisor(starting_index + 1, 1);
	glVertexAttribDivisor(starting_index + 2, 1);
	glVertexAttribDivisor(starting_index + 3, 1);
}

dodo::instanced::array_structs::position::position(glm::vec3 pos_vec)
	: pos_vec(pos_vec)
{
}

void dodo::instanced::array_structs::position::bind(GLuint starting_index)
{
	glEnableVertexAttribArray(starting_index);
	glVertexAttribPointer(starting_index, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec3), (void*)(0));
	glVertexAttribDivisor(starting_index, 1);
}
