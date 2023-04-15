#pragma once
#include "vulkan/vulkan.h"
#include "VulkanDevice.h"

namespace mg
{
	namespace commands
	{
		void drawIndexedIndirect(
			VkBuffer* vertexBuffer,
			VkBuffer* instanceBuffer,
			VkBuffer* indexBuffer,

			VkBuffer* indirectBuffer,
			uint32_t indirectDrawCount,

			VulkanDevice* vulkanDevice,
			VkCommandBuffer cmd,
			VkPipeline pipeline);

		void BeginCommandBuffer(VkCommandBuffer commandBuffer);


	}
}

