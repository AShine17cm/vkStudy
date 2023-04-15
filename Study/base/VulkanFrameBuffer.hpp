#pragma once

#include <array>
#include "vulkan/vulkan.h"
#include "VulkanDevice.h"
#include "VulkanTools.h"
#include "Buffer.h"

namespace mg
{
	struct FramebufferAttachment
	{
		VkImage image;
		VkDeviceMemory memory;
		VkImageView view;
		VkFormat format;
		VkImageSubresourceRange subresourceRange;
		VkAttachmentDescription	description;

		bool isDepth() {
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

		bool isStencil() {
			std::vector<VkFormat> formats = {
				VK_FORMAT_S8_UINT,
				VK_FORMAT_D16_UNORM_S8_UINT,
				VK_FORMAT_D24_UNORM_S8_UINT,
				VK_FORMAT_D32_SFLOAT_S8_UINT,
			};
			return std::find(formats.begin(), formats.end(), format) != std::end(formats);
		}

		bool hasDepthStencil() {
			return isDepth() || isStencil();
		}
	};

	struct mgAttachmentCreateInfo
	{
		uint32_t width, height;
		uint32_t layerCount;
		VkFormat format;
		VkImageUsageFlags usage;
		VkSampleCountFlagBits imageSampleCount = VK_SAMPLE_COUNT_1_BIT;
	};

	struct Framebuffer
	{
	private:
		VulkanDevice* device;
	public:
		uint32_t width, height;
		VkFramebuffer framebuffer;
		VkRenderPass renderPass;
		VkSampler sampler;
		std::vector<FramebufferAttachment> attachments;

		Framebuffer(VulkanDevice* device) {
			assert(device);
			this->device = device;
		}
		~Framebuffer() {
			assert(device);
			for (auto attachment : attachments)
			{
				vkDestroyImage(device->logicalDevice, attachment.image, nullptr);
				vkDestroyImageView(device->logicalDevice, attachment.view, nullptr);
				vkFreeMemory(device->logicalDevice, attachment.memory, nullptr);
			}
			vkDestroySampler(device->logicalDevice, sampler, nullptr);
			vkDestroyRenderPass(device->logicalDevice, renderPass, nullptr);
			vkDestroyFramebuffer(device->logicalDevice, framebuffer, nullptr);
		}
		void createFramebuffer() {

			uint32_t count = attachments.size();
			std::vector<VkImageView> imageViews(count);
			for (int i = 0; i < count; i++) {
				imageViews.push_back(attachments[i].view);
			}

			VkFramebufferCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			createInfo.width = width;
			createInfo.height = height;
			createInfo.layers = 1;
			createInfo.renderPass = renderPass;
			createInfo.attachmentCount = count;
			createInfo.pAttachments = imageViews.data();

			vkCreateFramebuffer(device->logicalDevice, &createInfo, nullptr, &framebuffer);

		}

		VkResult createRenderPass() {
			VkDevice logicalDevice = device->logicalDevice;
			uint32_t count = attachments.size();
			std::vector<VkAttachmentDescription> descriptions(count);
			for (int i = 0; i < count; i++) {
				descriptions[i] = attachments[i].description;
				//descriptions.push_back(attachments[i].description);
			};

			//Subpass	颜色相关的 attachment
			std::vector<VkAttachmentReference> colorRefs;
			VkAttachmentReference depthRef{};

			bool hasDepth = false;
			bool hasColor = false;
			uint32_t attachmentIndex = 0;
			for (auto& attachment : attachments) {
				if (attachment.hasDepthStencil()) {
					// Only one depth attachment allowed
					assert(!hasDepth);
					depthRef.attachment = attachmentIndex;
					depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					hasDepth = true;
				}
				else
				{
					colorRefs.push_back({ attachmentIndex,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
					hasColor = true;
				}
				attachmentIndex += 1;
			};
			//Subpass	默认只有一个
			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			if (hasColor) {
				subpass.colorAttachmentCount = colorRefs.size();
				subpass.pColorAttachments = colorRefs.data();
			}
			if (hasDepth) {
				subpass.pDepthStencilAttachment = &depthRef;
			}

			// Use subpass dependencies for attachment layout transitions
			std::array<VkSubpassDependency, 2> dependencies;
			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			dependencies[1].srcSubpass = 0;
			dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			//创建 RenderPass
			VkRenderPassCreateInfo renderPassCI{};
			renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassCI.attachmentCount = count;
			renderPassCI.pAttachments = descriptions.data();
			renderPassCI.subpassCount = 1;
			renderPassCI.pSubpasses = &subpass;
			renderPassCI.dependencyCount = dependencies.size();
			renderPassCI.pDependencies = dependencies.data();
			MG_CHECK_RESULT(vkCreateRenderPass(logicalDevice, &renderPassCI, nullptr, &renderPass));

			//创建 FrameBuffer
			std::vector<VkImageView> views;
			for (const auto att : attachments) {
				views.push_back(att.view);
			}
			//最大层数
			uint32_t maxLayers = 0;
			for (const auto& att : attachments) {
				if (att.subresourceRange.layerCount > maxLayers) {
					maxLayers = att.subresourceRange.layerCount;
				}
			}
			VkFramebufferCreateInfo framebufferCI{};
			framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferCI.width = width;
			framebufferCI.height = height;
			framebufferCI.layers = maxLayers;
			framebufferCI.attachmentCount = views.size();
			framebufferCI.pAttachments = views.data();
			framebufferCI.renderPass = renderPass;

			MG_CHECK_RESULT(vkCreateFramebuffer(logicalDevice, &framebufferCI, nullptr, &framebuffer));

			return VK_SUCCESS;
		}

		//index of attachment
		uint32_t addAttachment(mgAttachmentCreateInfo createInfo) {

			VkDevice logicalDevice = device->logicalDevice;
			FramebufferAttachment attachment;
			attachment.format = createInfo.format;
			//创建 Image
			VkImageCreateInfo imageCI{};
			imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageCI.arrayLayers = createInfo.layerCount;
			imageCI.format = createInfo.format;
			imageCI.samples = createInfo.imageSampleCount;
			imageCI.usage = createInfo.usage;
			imageCI.extent = { createInfo.width,createInfo.height,1 };
			imageCI.mipLevels = 1;
			imageCI.imageType = VK_IMAGE_TYPE_2D;
			imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
			//imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			//imageCI.initialLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;

			MG_CHECK_RESULT(vkCreateImage(logicalDevice, &imageCI, nullptr, &attachment.image));

			VkMemoryRequirements memReqs;
			vkGetImageMemoryRequirements(logicalDevice, attachment.image, &memReqs);

			VkMemoryAllocateInfo memoryAI{};
			memoryAI.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memoryAI.allocationSize = memReqs.size;
			memoryAI.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			MG_CHECK_RESULT(vkAllocateMemory(logicalDevice, &memoryAI, nullptr, &attachment.memory));
			MG_CHECK_RESULT(vkBindImageMemory(logicalDevice, attachment.image, attachment.memory, 0));


			// Color attachment,Depth (and/or stencil) attachment
			VkImageAspectFlags aspect = VK_FLAGS_NONE;
			if (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT & createInfo.usage) {
				aspect = VK_IMAGE_ASPECT_COLOR_BIT;
			}
			if (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT & createInfo.usage) {
				if (attachment.isDepth()) {
					aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
				}
				if (attachment.isStencil()) {
					aspect = aspect | VK_IMAGE_ASPECT_STENCIL_BIT;
				}
			}
			assert(aspect > 0);
			VkImageSubresourceRange range{};
			range.layerCount = createInfo.layerCount;
			range.aspectMask = aspect;
			range.levelCount = 1;

			attachment.subresourceRange = range;
			//创建 ImageView
			VkImageViewCreateInfo viewCI{};
			viewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewCI.image = attachment.image;
			viewCI.viewType = (createInfo.layerCount == 1) ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			viewCI.format = createInfo.format;
			viewCI.subresourceRange = range;
			viewCI.subresourceRange.aspectMask = attachment.isDepth() ? VK_IMAGE_ASPECT_DEPTH_BIT : aspect;

			MG_CHECK_RESULT(vkCreateImageView(logicalDevice, &viewCI, nullptr, &attachment.view));

			VkAttachmentDescription attDescription{};
			attDescription.format = createInfo.format;
			attDescription.samples = createInfo.imageSampleCount;
			attDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attDescription.finalLayout = attachment.hasDepthStencil() ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			attDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attDescription.storeOp = (createInfo.usage & VK_IMAGE_USAGE_SAMPLED_BIT) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

			if (attachment.hasDepthStencil()) {
				attDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			}
			else
			{
				attDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			}

			attachment.description = attDescription;
			attachments.push_back(attachment);
			return attachments.size() - 1;
		}



		VkResult createSampler(VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode addressMode) {
			VkSamplerCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			createInfo.addressModeU = addressMode;
			createInfo.addressModeV = addressMode;
			createInfo.addressModeW = addressMode;
			createInfo.minFilter = minFilter;
			createInfo.magFilter = magFilter;
			createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			createInfo.minLod = 0.0f;
			createInfo.maxLod = 1.0f;
			createInfo.anisotropyEnable = VK_TRUE;
			createInfo.maxAnisotropy = 1.0f;
			createInfo.mipLodBias = 0.0f;
			createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

			return vkCreateSampler(device->logicalDevice, &createInfo, nullptr, &sampler);
		}
	};
}

