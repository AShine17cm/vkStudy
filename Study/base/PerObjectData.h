#pragma once
#include "glm.hpp"
namespace geos 
{
	/* �� PushConstant������ */
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