#pragma once
#include "glm.hpp"
/* �� PushConstant������ */
struct PerObjectData
{
	glm::mat4 model;
	glm::vec4 texIndex;
};
