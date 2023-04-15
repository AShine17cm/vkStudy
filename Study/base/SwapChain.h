#pragma once
#include "vulkan/vulkan.h"
#include <vector>
//#include "textures.h"
namespace mg 
{
	struct SwapChain
	{
		uint32_t imageCount;
		VkExtent2D extent;
		VkFormat imageFormat;

		VkSwapchainKHR swapChain;
		std::vector<VkImage> images;
		std::vector<VkImageView> imageViews;

		SwapChain(uint32_t width, uint32_t height);

		void createSwapChain(VkPhysicalDevice physicalDevice,VkDevice device, VkSurfaceKHR surface);
		void cleanup(VkDevice device);
		//void createFramebuffers(VkDevice device, VkRenderPass renderPass,VkFramebuffer* framebuffers);
		//std::vector<VkFramebuffer> swapChainFramebuffers;  frame-buffer 和 render-pass相互关联
	};
}