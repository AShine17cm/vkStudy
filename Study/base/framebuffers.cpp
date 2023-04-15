#include "VulkanDevice.h"
#include "VulkanTools.h"
//#include "Texture.h"
#include "Framebuffers.h"
#include "VulkanFrameBuffer.hpp"

namespace mg
{
	namespace framebuffers
	{
		bool hasDepth(VkFormat format) {
			std::vector<VkFormat> formats = {
				VK_FORMAT_D32_SFLOAT_S8_UINT,
				VK_FORMAT_D32_SFLOAT,
				VK_FORMAT_D24_UNORM_S8_UINT,
				VK_FORMAT_D16_UNORM_S8_UINT,
				VK_FORMAT_D16_UNORM,
				VK_FORMAT_X8_D24_UNORM_PACK32,
			};
			return std::find(formats.begin(), formats.end(), format) != std::end(formats);
		}
		bool hasStencil(VkFormat format) {
			std::vector<VkFormat> formats = {
				VK_FORMAT_S8_UINT,
				VK_FORMAT_D16_UNORM_S8_UINT,
				VK_FORMAT_D24_UNORM_S8_UINT,
				VK_FORMAT_D32_SFLOAT_S8_UINT,
			};
			return std::find(formats.begin(), formats.end(), format) != std::end(formats);
		}
		bool isDepthStencil(VkFormat format) {
			return hasDepth(format) || hasStencil(format);
		}

		void createAttachment(
			uint32_t width,
			uint32_t height,
			VkFormat format,
			VkImageUsageFlags usage,
			VulkanDevice* device,

			VkImage* image,
			VkDeviceMemory* imageMemory,
			VkImageView* imageView,
			VkAttachmentDescription* attDescription)
		{
			uint32_t layerCount = 1;
			VkSampleCountFlagBits imageSampleCount = VK_SAMPLE_COUNT_1_BIT;

			VkDevice logicalDevice = device->logicalDevice;
			// Color attachment,Depth (and/or stencil) attachment
			VkImageAspectFlags aspect = VK_FLAGS_NONE;
			VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			if (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT & usage) {
				aspect = VK_IMAGE_ASPECT_COLOR_BIT;
				imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}
			if (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT & usage) {
				imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				if (hasDepth(format)) {
					aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
				}
				if (hasStencil(format)) {
					aspect = aspect | VK_IMAGE_ASPECT_STENCIL_BIT;
				}
			}
			assert(aspect > 0);
			//创建 Image
			VkImageCreateInfo imageCI{};
			imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageCI.arrayLayers = layerCount;
			imageCI.format = format;
			imageCI.samples = imageSampleCount;
			imageCI.usage = usage;
			imageCI.extent = { width,height,1 };
			imageCI.mipLevels = 1;
			imageCI.imageType = VK_IMAGE_TYPE_2D;
			imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
			//imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			//imageCI.initialLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;

			MG_CHECK_RESULT(vkCreateImage(logicalDevice, &imageCI, nullptr, image));

			VkMemoryRequirements memReqs;
			vkGetImageMemoryRequirements(logicalDevice, *image, &memReqs);

			VkMemoryAllocateInfo memoryAI{};
			memoryAI.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memoryAI.allocationSize = memReqs.size;
			memoryAI.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			MG_CHECK_RESULT(vkAllocateMemory(logicalDevice, &memoryAI, nullptr, imageMemory));
			MG_CHECK_RESULT(vkBindImageMemory(logicalDevice, *image, *imageMemory, 0));

			if (nullptr != imageView)
			{
				VkImageSubresourceRange range{};
				range.layerCount = layerCount;
				range.aspectMask = aspect;
				range.levelCount = 1;
				//创建 ImageView
				VkImageViewCreateInfo viewCI{};
				viewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				viewCI.image = *image;
				viewCI.viewType = (layerCount == 1) ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
				viewCI.format = format;
				viewCI.subresourceRange = range;

				MG_CHECK_RESULT(vkCreateImageView(logicalDevice, &viewCI, nullptr, imageView));
			}
			if (nullptr != attDescription)
			{
				attDescription->format = format;
				attDescription->samples = imageSampleCount;
				attDescription->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				attDescription->finalLayout = isDepthStencil(format) ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

				attDescription->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attDescription->storeOp = (usage & VK_IMAGE_USAGE_SAMPLED_BIT) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attDescription->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attDescription->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

				if (isDepthStencil(format)) {
					attDescription->finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
				}
				else
				{
					attDescription->finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				}
			}
		}

		void createAttachment(uint32_t width,
			uint32_t height,
			VkFormat format,
			VkImageUsageFlags usage,
			VulkanDevice* device,

			FramebufferAttachment* attachment)
		{
			createAttachment(
				width, height, format, usage, device,
				&attachment->image,
				&attachment->memory,
				&attachment->view,
				&attachment->description);
		}

		void createFramebuffers(
			VkExtent2D extent,
			VkRenderPass renderPass,
			VkImageView* attachments,
			uint32_t attachmentCount,
			VkDevice device,
			VkFramebuffer* framebuffer)
		{
			VkFramebufferCreateInfo framebufferCI{};
			framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferCI.width =extent.width;
			framebufferCI.height =extent.height;
			framebufferCI.renderPass = renderPass;
			framebufferCI.attachmentCount = attachmentCount;
			framebufferCI.pAttachments = attachments;
			framebufferCI.layers = 1;

			MG_CHECK_RESULT(vkCreateFramebuffer(device, &framebufferCI, nullptr, framebuffer));
		}

	}
}