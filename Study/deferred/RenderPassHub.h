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

	VkRenderPass shadowPass;
	VkRenderPass geo_mrt_Pass;
	VkRenderPass deferredPass;

	VkFramebuffer shadow_FB;
	VkFramebuffer geometry_FB;		//GBuffer: normal,pos,albedo

	std::vector<VkFramebuffer> swapChainFramebuffers;

	mg::SwapChain* swapchain;

	VkExtent2D extent;
	VkExtent2D shadowExtent;
	VkExtent2D gBufferExtent;

	void init(VulkanDevice* vulkanDevice,VkSurfaceKHR surface, VkFormat depthFormat) 
	{
		this->vulkanDevice = vulkanDevice;
		this->device = vulkanDevice->logicalDevice;

		extent = { WIDTH,HEIGHT };
		swapchain = new mg::SwapChain(WIDTH, HEIGHT);
		swapchain->createSwapChain(vulkanDevice->physicalDevice, device, surface);
	
		mg::renderpasses::create_PostProcess(swapchain->imageFormat, device, &deferredPass);
		/* 渲染Shadow-Map 所用的 RenderPass */
		mg::renderpasses::createDepthRenderPass(depthFormat, device, &shadowPass);
		/* 渲染 G-Buffer 所用的 RenderPass */
		std::array<VkFormat, 4> formats = {
			VK_FORMAT_R16G16B16A16_SFLOAT ,
			VK_FORMAT_R16G16B16A16_SFLOAT ,
			VK_FORMAT_R8G8B8A8_UNORM,
			depthFormat };
		mg::renderpasses::create_MRT(formats.data(), device, &geo_mrt_Pass);
	}

	void createFrameBuffers(Resource* res) 
	{
		/* Framebuffers */
		swapChainFramebuffers.resize(swapchain->imageCount);
		uint32_t attachmentCount = 1;//延迟渲染 着色阶段不需要 depth
		for (uint32_t i = 0; i < swapchain->imageCount; i++)
		{
			VkImageView images[] = { swapchain->imageViews[i] };
			framebuffers::createFramebuffers(swapchain->extent, deferredPass, images, attachmentCount,device, &swapChainFramebuffers[i]);
		}
		/* 阴影相关的 FrameBuffer */
		shadowExtent = res->tex_shadow->info.getExtend2D();
		VkImageView shadowViews[] = { res->tex_shadow->view };
		framebuffers::createFramebuffers(shadowExtent, shadowPass, shadowViews, 1,device, &shadow_FB,LIGHT_COUNT);

		/* 延迟渲染 G-Buffer */
		gBufferExtent = res->geo_pos->info.getExtend2D();
		VkImageView geoViews[] = { 
			res->geo_pos->view,
			res->geo_normal->view,
			res->geo_albedo->view,
			res->geo_depth->view };//+1
		framebuffers::createFramebuffers(gBufferExtent, geo_mrt_Pass, geoViews, G_BUFFER_COUNT+1, device, &geometry_FB);
	}
	void clean() 
	{
		for (auto framebuffer : swapChainFramebuffers)
		{
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}
		vkDestroyFramebuffer(device, shadow_FB, nullptr);
		vkDestroyFramebuffer(device, geometry_FB, nullptr);

		swapchain->cleanup(device);
		vkDestroyRenderPass(device, shadowPass, nullptr);
		vkDestroyRenderPass(device, geo_mrt_Pass, nullptr);
		vkDestroyRenderPass(device, deferredPass, nullptr);
	}
};
