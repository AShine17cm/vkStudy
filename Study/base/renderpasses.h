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
	//���� ֻ��һ��depth attachment����� render-pass
	void createDepthRenderPass(
		VkFormat format,
		VkDevice vulkanDevice,
		VkRenderPass* renderPass,
		VkSampleCountFlagBits SAMPLE_COUNT = VK_SAMPLE_COUNT_1_BIT
	);

	//���� ֻ��һ��color attachment��render-pass
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
