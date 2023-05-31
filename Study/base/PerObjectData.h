#pragma once
#include "glm.hpp"
namespace geos 
{
	/* 走 PushConstant的数据 */
	struct PerObjectData
	{
		glm::mat4 model;
		glm::vec4 texIndex;
	};
	struct PbrBasic
	{
		glm::vec4 rgba;
		float roughness;
		float metallic;
	};
}