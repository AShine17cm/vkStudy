#include <vector>
#include "vertices.h"

namespace mg
{
	namespace vertexinputs
	{
		//offsets第一个元素是 stride
		VkPipelineVertexInputStateCreateInfo attributesInterleaved(
			VkFormat* formats,
			uint32_t* offsets,
			uint32_t attributeCount)
		{
			uint32_t idxBinding = 0;
			uint32_t stride = offsets[0];

			VkVertexInputBindingDescription binding = { idxBinding,stride,VK_VERTEX_INPUT_RATE_VERTEX };

			std::vector<VkVertexInputAttributeDescription> attributes;
			for (uint32_t i = 0; i < attributeCount; i++)
			{
				VkVertexInputAttributeDescription attribute =
				{ i,idxBinding,formats[i],offsets[i + 1] };

				attributes.push_back(attribute);
			}

			VkPipelineVertexInputStateCreateInfo vertexInputStateCI{};
			vertexInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputStateCI.vertexBindingDescriptionCount = 1;
			vertexInputStateCI.pVertexBindingDescriptions = &binding;
			vertexInputStateCI.vertexAttributeDescriptionCount = attributeCount;
			vertexInputStateCI.pVertexAttributeDescriptions = attributes.data();
			return vertexInputStateCI;
		}
	}
}