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
}