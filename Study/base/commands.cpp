#include <basetsd.h>
#include "commands.h"
#include "VulkanTools.h"
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
			VkPipeline pipeline)
		{
			ULONG64 stride = sizeof(VkDrawIndexedIndirectCommand);
			VkDeviceSize offset = 0;
			uint32_t firstBinding = 0;

			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
			// Binding point 0 : Mesh vertex buffer
			// Binding point 1 : Instance data buffer
			vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffer, &offset);
			vkCmdBindVertexBuffers(cmd, 1, 1, instanceBuffer, &offset);

			vkCmdBindIndexBuffer(cmd, *indexBuffer, offset, VK_INDEX_TYPE_UINT32);

			// If the multi draw feature is supported:
			// One draw call for an arbitrary number of objects
			// Index offsets and instance count are taken from the indirect buffer
			if (vulkanDevice->enabledFeatures.multiDrawIndirect) {
				vkCmdDrawIndexedIndirect(cmd, *indirectBuffer, offset, indirectDrawCount, stride);
			}
			else
			{
				uint32_t drawCount = 1;
				for (uint32_t d = 0; d < indirectDrawCount; d++) {

					vkCmdDrawIndexedIndirect(cmd, *indirectBuffer, d * stride, drawCount, stride);
				}
			}


		}

		void BeginCommandBuffer(VkCommandBuffer commandBuffer) 
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			MG_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));
		}


	}
}