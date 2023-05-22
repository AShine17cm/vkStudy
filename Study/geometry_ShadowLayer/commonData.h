#pragma once
#include "glm.hpp"
/* 走 PushConstant的数据 */
struct PerObjectData
{
	glm::mat4 model;
	glm::vec4 texIndex;
};
