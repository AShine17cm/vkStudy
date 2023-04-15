#pragma once
#include "vulkan/vulkan.h"

namespace mg
{
	namespace vertexinputs
	{
		// Interleaved vertex attributes 
		// One Binding (one buffer) and multiple attributes
		VkPipelineVertexInputStateCreateInfo attributesInterleaved(
			VkFormat* formats,
			uint32_t* offsets,
			uint32_t attributeCount);
	}
}
