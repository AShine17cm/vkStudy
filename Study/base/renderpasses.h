#pragma once
#include "framebuffers.h"
namespace mg 
{
namespace renderpasses 
{
	//创建 只有一个color attachment的render-pass
	void createRenderPass(
		VkFormat format,
		VkFormat depthFromat,
		VkDevice device,
		VkRenderPass* renderPass
	);
	//创建 只有一个depth attachment的深度 render-pass
	void createDepthRenderPass(
		VkFormat format,
		VkDevice vulkanDevice,
		VkRenderPass* renderPass
	);
}
}
