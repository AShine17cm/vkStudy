#pragma once
#include "framebuffers.h"
namespace mg 
{
namespace renderpasses 
{
	//���� ֻ��һ��color attachment��render-pass
	void createRenderPass(
		VkFormat format,
		VkFormat depthFromat,
		VkDevice device,
		VkRenderPass* renderPass
	);
	//���� ֻ��һ��depth attachment����� render-pass
	void createDepthRenderPass(
		VkFormat format,
		VkDevice vulkanDevice,
		VkRenderPass* renderPass
	);
}
}
