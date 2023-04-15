#pragma once
#include "vulkan/vulkan.h"
#include "VulkanDevice.h"
#include "VulkanFrameBuffer.hpp"
namespace mg
{
	namespace framebuffers
	{
		bool hasDepth(VkFormat format);
		bool hasStencil(VkFormat format);
		bool isDepthStencil(VkFormat format);

		void createAttachment(
			uint32_t width,
			uint32_t height,
			VkFormat format,
			VkImageUsageFlags usage,
			VulkanDevice* device,

			VkImage* image,
			VkDeviceMemory* imageMemory,
			VkImageView* imageView,
			VkAttachmentDescription* attDescription);

		void createAttachment(uint32_t width,
			uint32_t height,
			VkFormat format,
			VkImageUsageFlags usage,
			VulkanDevice* device,

			FramebufferAttachment* attachment);

		void createFramebuffers(
			VkExtent2D extent,
			VkRenderPass renderPass,
			VkImageView* attachments,
			uint32_t attachmentCount,
			VkDevice device,
			VkFramebuffer* framebuffer);

	}
}

