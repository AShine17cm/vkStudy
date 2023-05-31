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

		//高级参数
		float exposure;
		float gamma;
		float prefilteredCubeMipLevels;
		float scaleIBLAmbient;
		float debugViewInputs = 0;
		float debugViewEquation = 0;
	};
}