#include "batches.h"
#include "descriptors.h"
#include "VulkanTools.h"
#include "shaders.h"
#include "pipelines.h"
namespace mg 
{
namespace batches 
{
	void BeginRenderpass(
		VkCommandBuffer cmd,
		VkRenderPass renderPass,
		VkFramebuffer framebuffer,
		VkClearValue* clearValues,
		uint32_t clearCount,
		VkExtent2D extent
	)
	{
		VkRenderPassBeginInfo renderPassBI{};
		renderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBI.framebuffer = framebuffer;
		renderPassBI.renderPass = renderPass;
		renderPassBI.renderArea.offset = { 0,0 };
		renderPassBI.renderArea.extent = extent;
		renderPassBI.pClearValues = clearValues;
		renderPassBI.clearValueCount = clearCount;

		vkCmdBeginRenderPass(cmd, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);

	}

	//默认的窗口设置
	void SetViewport(
		VkCommandBuffer cmd,
		VkExtent2D extent)
	{
		VkViewport viewport{};
		viewport.width =extent.width;
		viewport.height =extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(cmd, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.extent = extent;
		scissor.offset = { 0,0 };
		vkCmdSetScissor(cmd, 0, 1, &scissor);
	}

}
}