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
		VkRenderPass* renderPass,
		VkSampleCountFlagBits SAMPLE_COUNT=VK_SAMPLE_COUNT_1_BIT
	);
	void create_PostProcess(
		VkFormat format,
		VkDevice device,
		VkRenderPass* renderPass,
		VkSampleCountFlagBits SAMPLE_COUNT = VK_SAMPLE_COUNT_1_BIT
	);
	void create_MRT(
		VkFormat* formats,
		VkDevice device,
		VkRenderPass* renderPass,
		VkSampleCountFlagBits SAMPLE_COUNT = VK_SAMPLE_COUNT_1_BIT
	);
	//创建 只有一个depth attachment的深度 render-pass
	void createDepthRenderPass(
		VkFormat format,
		VkDevice vulkanDevice,
		VkRenderPass* renderPass,
		VkSampleCountFlagBits SAMPLE_COUNT = VK_SAMPLE_COUNT_1_BIT
	);

	//创建 只有一个color attachment的render-pass
	void createRenderPass_MSAA(
		VkFormat format,
		VkFormat depthFromat,
		VkDevice device,
		VkRenderPass* renderPass,
		VkSampleCountFlagBits SAMPLE_COUNT = VK_SAMPLE_COUNT_4_BIT,
		VkBool32 isScreen=VK_TRUE
	);

	void createRenderPass_HDR(
		VkFormat format,
		VkFormat depthFromat,
		VkDevice device,
		VkRenderPass* renderPass
	);

	void createRenderPass_Bloom(
		VkFormat format,
		VkDevice device,
		VkRenderPass* renderPass
	);
}
}
