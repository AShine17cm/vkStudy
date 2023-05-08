#pragma once
#include "commands.h"
#include <vector>
#include "vulkan/vulkan.h"
#include "Resource.h"


using namespace glm;
using namespace mg;
/*	RenderPass 
	与RenderPass绑定的 FrameBuffer-ImageView
	同一个 Depth-Tex 被多个 FrameBuffer 使用
*/
struct RenderPassHub
{
	VulkanDevice* vulkanDevice;
	VkDevice device;

	VkRenderPass renderPass;
	VkRenderPass shadowPass;

	VkFramebuffer shadow_FB;
	std::vector<VkFramebuffer> swapChainFramebuffers;

	mg::SwapChain* swapchain;

	VkExtent2D extent;
	VkExtent2D shadowExtent;

	void init(VulkanDevice* vulkanDevice,VkSurfaceKHR surface, VkFormat depthFormat) 
	{
		this->vulkanDevice = vulkanDevice;
		this->device = vulkanDevice->logicalDevice;

		extent = { WIDTH,HEIGHT };
		swapchain = new mg::SwapChain(WIDTH, HEIGHT);
		swapchain->createSwapChain(vulkanDevice->physicalDevice, device, surface);
	
		mg::renderpasses::createRenderPass(swapchain->imageFormat, depthFormat, device, &renderPass);
		/* 渲染Shadow-Map 所用的 RenderPass */
		mg::renderpasses::createDepthRenderPass(depthFormat, device, &shadowPass);
	}

	void createFrameBuffers(Resource* res) 
	{
		/* Framebuffers */
		swapChainFramebuffers.resize(swapchain->imageCount);
		uint32_t attachmentCount = 2;
		for (uint32_t i = 0; i < swapchain->imageCount; i++)
		{
			VkImageView images[] = { swapchain->imageViews[i],res->tex_depth->view };//depth 共享一个
			framebuffers::createFramebuffers(swapchain->extent, renderPass, images, attachmentCount,device, &swapChainFramebuffers[i]);
		}
		/* 阴影相关的 FrameBuffer */
		shadowExtent = res->tex_shadow->info.getExtend2D();
		VkImageView shadowViews[] = { res->tex_shadow->view };
		framebuffers::createFramebuffers(shadowExtent, shadowPass, shadowViews, 1,device, &shadow_FB,LIGHT_COUNT);
	}
	void clean() 
	{
		for (auto framebuffer : swapChainFramebuffers)
		{
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}
		vkDestroyFramebuffer(device, shadow_FB, nullptr);

		swapchain->cleanup(device);
		vkDestroyRenderPass(device, renderPass, nullptr);
		vkDestroyRenderPass(device, shadowPass, nullptr);
	}
};
