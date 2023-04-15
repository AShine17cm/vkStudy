//#include <exception>
//#include "vulkan/vulkan.h"
//#include "mg/mgVulkanDevice.h"
#include "textures.h"
#include "VulkanTools.h"
//#include "framebuffers.h"
namespace mg
{
namespace textures
{

		void createTexture(
			MgImageInfo info,
			uint32_t size,
			VulkanDevice* device,
			VkImage* image,
			VkDeviceMemory* imageMemory,
			void* texData,
			uint32_t* offsets,
			MgTextureEx* extends,
			uint32_t exCount)
		{

			VkDevice logicalDevice = device->logicalDevice;
			VkCommandBuffer cmdBuffer = device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

			VkMemoryRequirements memReqs;
			VkMemoryAllocateInfo memAI{};
			VkImageCreateInfo imageCI{};

			/* Image 创建 */
			imageCI.sType			= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageCI.arrayLayers		= info.layers;
			imageCI.mipLevels		= info.mipLevels;
			imageCI.extent			= info.extent3D;
			imageCI.imageType		= info.formats.type;
			imageCI.format			= info.formats.format;
			imageCI.usage			= info.formats.usageFlags | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			imageCI.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;//VK_IMAGE_LAYOUT_PREINITIALIZED
			imageCI.samples			= VK_SAMPLE_COUNT_1_BIT;
			imageCI.sharingMode		= VK_SHARING_MODE_EXCLUSIVE;
			imageCI.tiling			= VK_IMAGE_TILING_OPTIMAL;

			//检查扩展
			for (uint32_t i = 0; i < exCount; i++) 
			{
				MgTextureEx exKind = extends[i];
				switch (exKind)
				{
				case MgTextureEx::Image_Cube:
					imageCI.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;//This flag required for cube map images
					break;
				}
			}

			MG_CHECK_RESULT(vkCreateImage(logicalDevice, &imageCI, nullptr, image));
			vkGetImageMemoryRequirements(logicalDevice, *image, &memReqs);
			memAI.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memAI.allocationSize = memReqs.size;
			memAI.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			MG_CHECK_RESULT(vkAllocateMemory(logicalDevice, &memAI, nullptr, imageMemory));
			MG_CHECK_RESULT(vkBindImageMemory(logicalDevice, *image, *imageMemory, 0));
			/* image创建 */
			if (nullptr == texData)
			{
				//12.26
				//bool hasDepth = framebuffers::hasDepth(info.formats.format);
				////VkImageAspectFlags aspect = hasDepth ? VK_IMAGE_ASPECT_DEPTH_BIT|VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
				//if(!hasDepth)
				//{
				//	VkImageSubresourceRange sRange = { VK_IMAGE_ASPECT_COLOR_BIT,0,info.mipLevels,0,info.layers };
				//	setImageLayout(cmdBuffer, *image, VK_IMAGE_LAYOUT_UNDEFINED, info.formats.imagelayout, &sRange);
				//	device->flushCommandBuffer(cmdBuffer, device->graphicsQueue);
				//}
				return;
			}


			/* 使用 stagingBuffer */
			VkBuffer		stagingBuffer;
			VkDeviceMemory	stagingMemory;
			VkBufferCreateInfo bufferCI{};
			bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			bufferCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			bufferCI.size = size;

			MG_CHECK_RESULT(vkCreateBuffer(logicalDevice, &bufferCI, nullptr, &stagingBuffer));
			vkGetBufferMemoryRequirements(logicalDevice, stagingBuffer, &memReqs);
			memAI.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memAI.allocationSize = memReqs.size;
			memAI.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			MG_CHECK_RESULT(vkAllocateMemory(logicalDevice, &memAI, nullptr, &stagingMemory));
			MG_CHECK_RESULT(vkBindBufferMemory(logicalDevice, stagingBuffer, stagingMemory, 0));

			void* mapped;
			MG_CHECK_RESULT(vkMapMemory(logicalDevice, stagingMemory, 0, memReqs.size, 0, &mapped));
			memcpy(mapped, texData, size);
			vkUnmapMemory(logicalDevice, stagingMemory);
			/* 使用staging - buffer */

			VkImageSubresourceRange subresourceRange= { VK_IMAGE_ASPECT_COLOR_BIT,0,info.mipLevels,0,info.layers };
			std::vector<VkBufferImageCopy> copys;
			//copys 和ktx相关的内存布局
			for (uint32_t layer = 0; layer < info.layers; layer++)
			{
				for (uint32_t level = 0; level < info.mipLevels; level++)
				{
					VkBufferImageCopy copy{};
					copy.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT,level,layer,1 };
					copy.imageExtent = { info.extent3D.width >> level,info.extent3D.height >> level,1 };
					copy.bufferOffset = offsets[layer * info.mipLevels + level];
					copys.push_back(copy);
				}
			}
			//从stagingBuffer拷贝,优化Image-layout
			setImageLayout(cmdBuffer, *image,VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,&subresourceRange);
			vkCmdCopyBufferToImage(cmdBuffer, stagingBuffer, *image,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,copys.size(), copys.data());
			//拷贝完成，重新优化image layout
			setImageLayout(cmdBuffer, *image,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,info.formats.imagelayout,&subresourceRange);
			device->flushCommandBuffer(cmdBuffer,device->graphicsQueue);

			vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
			vkFreeMemory(logicalDevice, stagingMemory, nullptr);
		}
		//内存大小 通过image的格式计算出来
		void createTexture_Simple(
			MgImageInfo info,
			void* texData,
			VulkanDevice* device,
			VkImage* image,
			VkDeviceMemory* imageMemory,
			MgTextureEx* extFeatures,
			uint32_t extCount)
		{
			// Prefer using optimal tiling, as linear tiling 
			// may support only a small set of features 
			// depending on implementation (e.g. no mip maps, only one layer, etc.)

			VkDevice logicalDevice = device->logicalDevice;
			VkCommandBuffer cmdBuffer = device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

			VkImageCreateInfo imageCI{};
			VkMemoryRequirements memReqs{};
			VkMemoryAllocateInfo memoryAI{};

			/* image创建 */
			imageCI.sType		=	VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageCI.extent		=	info.extent3D;
			imageCI.arrayLayers =	info.layers;
			imageCI.mipLevels	=	info.mipLevels;
			imageCI.format		=	info.formats.format;
			imageCI.usage		=	info.formats.usageFlags;
			imageCI.imageType	=	info.formats.type;
			imageCI.sharingMode =	VK_SHARING_MODE_EXCLUSIVE;
			imageCI.samples		=	VK_SAMPLE_COUNT_1_BIT;
			imageCI.tiling		=	VK_IMAGE_TILING_LINEAR;
			imageCI.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;// VK_IMAGE_LAYOUT_UNDEFINED;

			//检查 额外扩展
			for (uint32_t i = 0; i < extCount; i++)
			{
				MgTextureEx extend = extFeatures[i];
				switch (extend)
				{
				case MgTextureEx::Image_Cube:
					imageCI.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;//This flag is required for cube map images
					break;
				}
			}
			MG_CHECK_RESULT(vkCreateImage(logicalDevice, &imageCI, nullptr, image));
			vkGetImageMemoryRequirements(logicalDevice, *image, &memReqs);
			memoryAI.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memoryAI.allocationSize = memReqs.size;
			memoryAI.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			MG_CHECK_RESULT(vkAllocateMemory(logicalDevice, &memoryAI, nullptr, imageMemory));
			MG_CHECK_RESULT(vkBindImageMemory(logicalDevice, *image, *imageMemory, 0));
			/* image创建 */
			if (nullptr == texData)return;

			//拷贝数据
			void* mapped;
			vkMapMemory(logicalDevice, *imageMemory, 0, memReqs.size, 0, &mapped);
			memcpy(mapped, texData, memReqs.size);
			vkUnmapMemory(logicalDevice, *imageMemory);

			VkImageSubresourceRange range = { VK_IMAGE_ASPECT_COLOR_BIT,0,info.mipLevels,0,info.layers };
			textures::setImageLayout(cmdBuffer,*image,VK_IMAGE_LAYOUT_PREINITIALIZED,info.formats.imagelayout,&range);//LAYOUT_UNDEFINED,
			device->flushCommandBuffer(cmdBuffer, device->graphicsQueue);
		}
		/*
		根据Format 检测color和depth的aspectMask
		*/
		void createImageView(
			VkDevice device,
			VkImage image,
			VkImageView* view,
			MgImageInfo extent,
			VkImageViewType viewType)
		{
			VkImageViewCreateInfo viewCI{};
			viewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewCI.format =extent.formats.format;
			viewCI.image = image;
			viewCI.components = { VK_COMPONENT_SWIZZLE_R , VK_COMPONENT_SWIZZLE_G , VK_COMPONENT_SWIZZLE_B , VK_COMPONENT_SWIZZLE_A };
			viewCI.viewType = viewType;
			viewCI.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT,0,extent.mipLevels,0,extent.layers };

			switch (extent.formats.format)
			{
			case VK_FORMAT_D16_UNORM:
			case VK_FORMAT_D16_UNORM_S8_UINT:
			case VK_FORMAT_D24_UNORM_S8_UINT:
			case VK_FORMAT_D32_SFLOAT:
			case VK_FORMAT_D32_SFLOAT_S8_UINT:
				viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
				break;
			default:
				break;
			}

			MG_CHECK_RESULT(vkCreateImageView(device, &viewCI, nullptr, view));
		}
		void createSampler(
			VulkanDevice* device,
			uint32_t mipLevels,
			VkSampler* sampler,
			MgTextureEx* extends,
			uint32_t extendCount,
			VkSamplerAddressMode addressMode)
		{
			VkDevice logicalDevice = device->logicalDevice;
			VkPhysicalDeviceFeatures enabledFeatures = device->enabledFeatures;
			VkSamplerCreateInfo samplerCI{};
			samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerCI.addressModeU = addressMode;
			samplerCI.addressModeV = addressMode;
			samplerCI.addressModeW = addressMode;
			samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
			samplerCI.minFilter = VK_FILTER_LINEAR;
			samplerCI.magFilter = VK_FILTER_LINEAR;
			samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerCI.mipLodBias = 0.0f;
			samplerCI.minLod = 0.0f;
			samplerCI.maxLod = (float)mipLevels;
			//samplerCI.compareEnable = VK_FALSE;
			samplerCI.compareOp = VK_COMPARE_OP_NEVER;

			//检查扩展
			for (uint32_t i = 0; i < extendCount; i++)
			{
				MgTextureEx extend = extends[i];
				switch (extend)
				{
				case MgTextureEx::None:
				case MgTextureEx::Sampler_None:
					samplerCI.anisotropyEnable = enabledFeatures.samplerAnisotropy;
					samplerCI.maxAnisotropy = enabledFeatures.samplerAnisotropy ? device->properties.limits.maxSamplerAnisotropy : 1.0f;
					break;
				case MgTextureEx::Sampler_AnisotropyDisable://关闭抗锯齿
					samplerCI.maxAnisotropy = 1.0f;
					samplerCI.anisotropyEnable = VK_FALSE;
					break;
				case MgTextureEx::Sampler_BorderWhite:
					samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
					break;
				}
			}
			MG_CHECK_RESULT(vkCreateSampler(logicalDevice, &samplerCI, nullptr, sampler));
		}
		/*
		从image-Layout 确定 内存access类型
		cmd-pipeline-barrier
		*/
		void setImageLayout(
			VkCommandBuffer cmdBuffer,
			VkImage image,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkImageSubresourceRange* subresourceRange,
			VkPipelineStageFlags srcStageMask,
			VkPipelineStageFlags dstStageMask)
		{

			VkAccessFlags srcAccess = 0;
			VkAccessFlags dstAccess = 0;
			switch (oldImageLayout)
			{
			case VK_IMAGE_LAYOUT_UNDEFINED:
				// Image layout is undefined (or does not matter)
				// Only valid as initial layout
				// No flags required, listed only for completeness
				srcAccess = 0;
				break;
			case VK_IMAGE_LAYOUT_PREINITIALIZED:
				//保证所有 CPU写操作已经完成
				srcAccess = VK_ACCESS_HOST_WRITE_BIT;
				break;
			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				//保证上一阶段 写操作完成
				srcAccess = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				break;
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				//保证上一阶段 写操作完成
				srcAccess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				break;
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				//保证上一阶段 写操作完成
				srcAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
				break;
			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				//保证上一阶段 读操作完成
				srcAccess = VK_ACCESS_TRANSFER_READ_BIT;
				break;
			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				//保证上一阶段 读操作完成
				srcAccess = VK_ACCESS_SHADER_READ_BIT;
				break;
				//12.26 offscreen 渲染，将color attach转为texture
			case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
				srcAccess = 0;
				break;
			default:
				throw std::exception("mg textures 我的错:: unsupport Image layout ");
				break;
			}
			switch (newImageLayout)
			{
			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				//等待上一阶段的 写操作完成
				dstAccess = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				break;
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				//等待上一阶段的 写操作完成
				dstAccess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				break;
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				//等待上一阶段的 写操作完成
				dstAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
				break;
			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				//等待上一阶段的 读操作完成
				dstAccess = VK_ACCESS_TRANSFER_READ_BIT;
				break;
			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				// Make sure any writes to the image have been finished
				if (srcAccess == 0)
				{
					srcAccess = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
				}
				//等待上一阶段的 读操作完成
				dstAccess = VK_ACCESS_SHADER_READ_BIT;
				break;
				//12.26 offscreen 渲染，将depth attach转为texture
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
			//case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR:
				dstAccess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				break;
			default:
				throw std::exception("mg textures 我的错:: unsupport Image layout ");
				break;
			}
			//创建个 默认的subresource-range
			if (nullptr == subresourceRange) 
			{
				VkImageSubresourceRange subres = { VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1 };
				subresourceRange = &subres;
			}

			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.image = image;
			barrier.oldLayout = oldImageLayout;
			barrier.newLayout = newImageLayout;
			barrier.subresourceRange = *subresourceRange;
			barrier.srcAccessMask = srcAccess;
			barrier.dstAccessMask = dstAccess;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			vkCmdPipelineBarrier(cmdBuffer,
				srcStageMask, dstStageMask,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier);
		}


}
}