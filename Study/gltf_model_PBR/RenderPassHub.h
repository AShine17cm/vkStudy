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

	//MSAA 多出来的两块 透明内存
	struct MultisampleTarget {
		struct {
			VkImage image;
			VkImageView view;
			VkDeviceMemory memory;
		} color;
		struct {
			VkImage image;
			VkImageView view;
			VkDeviceMemory memory;
		} depth;
	} multisampleTarget;

	void init(VulkanDevice* vulkanDevice,VkSurfaceKHR surface, VkFormat depthFormat) 
	{
		this->vulkanDevice = vulkanDevice;
		this->device = vulkanDevice->logicalDevice;

		extent = { WIDTH,HEIGHT };
		swapchain = new mg::SwapChain(WIDTH, HEIGHT);
		swapchain->createSwapChain(vulkanDevice->physicalDevice, device, surface);

		setupMultisampleTarget(swapchain->imageFormat, depthFormat);				//MSAA 额外的 render target
		mg::renderpasses::createRenderPass_MSAA(swapchain->imageFormat, depthFormat, device, &renderPass);
		/* 渲染Shadow-Map 所用的 RenderPass */
		mg::renderpasses::createDepthRenderPass(depthFormat, device, &shadowPass);
	}

	void createFrameBuffers(Resource* res) 
	{
		/* Framebuffers */
		swapChainFramebuffers.resize(swapchain->imageCount);
		uint32_t attachmentCount = 4;
		for (uint32_t i = 0; i < swapchain->imageCount; i++)
		{
			VkImageView images[] =
			{
				multisampleTarget.color.view,
				swapchain->imageViews[i],
				multisampleTarget.depth.view,
				res->tex_depth->view
			};//depth 共享一个
			framebuffers::createFramebuffers(swapchain->extent, renderPass, images, attachmentCount,device, &swapChainFramebuffers[i]);
		}
		/* 阴影相关的 FrameBuffer */
		shadowExtent = res->tex_shadow->info.getExtend2D();
		VkImageView shadowViews[] = { res->tex_shadow->view };
		framebuffers::createFramebuffers(shadowExtent, shadowPass, shadowViews, 1,device, &shadow_FB,LIGHT_COUNT);
	}
	//抄一下 MSAA的代码
	void setupMultisampleTarget(VkFormat colorFormat, VkFormat depthFormat)
	{
		// Check if device supports requested sample count for color and depth frame buffer
		//assert((deviceProperties.limits.framebufferColorSampleCounts >= sampleCount) && (deviceProperties.limits.framebufferDepthSampleCounts >= sampleCount));

		// Color target
		VkImageCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.imageType = VK_IMAGE_TYPE_2D;
		info.format = colorFormat;
		info.extent.width = WIDTH;
		info.extent.height = HEIGHT;
		info.extent.depth = 1;
		info.mipLevels = 1;
		info.arrayLayers = 1;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.samples = VK_SAMPLE_COUNT_4_BIT;
		//片上缓存
		info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		MG_CHECK_RESULT(vkCreateImage(device, &info, nullptr, &multisampleTarget.color.image));

		VkMemoryRequirements memReqs;
		vkGetImageMemoryRequirements(device, multisampleTarget.color.image, &memReqs);
		VkMemoryAllocateInfo memAlloc{};
		memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAlloc.allocationSize = memReqs.size;
		// We prefer a lazily allocated memory type
		// This means that the memory gets allocated when the implementation sees fit, e.g. when first using the images
		VkBool32 lazyMemTypePresent;
		memAlloc.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT, &lazyMemTypePresent);
		if (!lazyMemTypePresent)
		{
			// If this is not available, fall back to device local memory
			memAlloc.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		}
		MG_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &multisampleTarget.color.memory));
		vkBindImageMemory(device, multisampleTarget.color.image, multisampleTarget.color.memory, 0);

		// Create image view for the MSAA target
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = multisampleTarget.color.image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = colorFormat;
		viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.layerCount = 1;

		MG_CHECK_RESULT(vkCreateImageView(device, &viewInfo, nullptr, &multisampleTarget.color.view));

		// Depth target
		info.imageType = VK_IMAGE_TYPE_2D;
		info.format = depthFormat;
		info.extent.width = WIDTH;
		info.extent.height = HEIGHT;
		info.extent.depth = 1;
		info.mipLevels = 1;
		info.arrayLayers = 1;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.samples = VK_SAMPLE_COUNT_4_BIT;
		//片上缓存
		info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		MG_CHECK_RESULT(vkCreateImage(device, &info, nullptr, &multisampleTarget.depth.image));

		vkGetImageMemoryRequirements(device, multisampleTarget.depth.image, &memReqs);
		memAlloc = {};
		memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAlloc.allocationSize = memReqs.size;

		memAlloc.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT, &lazyMemTypePresent);
		if (!lazyMemTypePresent)
		{
			memAlloc.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		}

		MG_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &multisampleTarget.depth.memory));
		vkBindImageMemory(device, multisampleTarget.depth.image, multisampleTarget.depth.memory, 0);

		// Create image view for the MSAA target
		viewInfo.image = multisampleTarget.depth.image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = depthFormat;
		viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT)
			viewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.layerCount = 1;

		MG_CHECK_RESULT(vkCreateImageView(device, &viewInfo, nullptr, &multisampleTarget.depth.view));
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

		vkDestroyImage(device, multisampleTarget.color.image, nullptr);
		vkDestroyImageView(device, multisampleTarget.color.view, nullptr);
		vkFreeMemory(device, multisampleTarget.color.memory, nullptr);
		vkDestroyImage(device, multisampleTarget.depth.image, nullptr);
		vkDestroyImageView(device, multisampleTarget.depth.view, nullptr);
		vkFreeMemory(device, multisampleTarget.depth.memory, nullptr);
	}
};
