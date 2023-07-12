#pragma once
#include "VirtualTexture.h"
#include <iostream>
#include "VulkanDevice.h"
#include "textures.h"
#include <cmath>

//#include <glm.hpp>

//将虚拟纹理 当 正常纹理使用
struct SparseTexture :VirtualTexture
{
	VkSampler sampler;
	VkImageLayout imageLayout;
	VkImageView view;
	VkDescriptorImageInfo descriptor;
	VkFormat format;
	uint32_t width, height;
	uint32_t mipLevels;
	uint32_t layerCount;
	VkImageSubresourceRange subRange;

	void destroyTextureImage( ) 
	{
		vkDestroyImageView(device, view, nullptr);
		vkDestroyImage(device, image, nullptr);
		vkDestroySampler(device, sampler, nullptr);
		//texture.destroy();
	}
};
struct VirtualTextureHub
{
	SparseTexture texture;
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	mg::VulkanDevice* vulkanDevice;
	VkQueue queue;
	VkSemaphore bindSparseSemaphore = VK_NULL_HANDLE;

	VirtualTextureHub(mg::VulkanDevice* vulkanDevice)
	{
		this->vulkanDevice = vulkanDevice;
		this->physicalDevice = vulkanDevice->physicalDevice;
		this->device = vulkanDevice->logicalDevice;
		this->queue = vulkanDevice->graphicsQueue;
	}
	void clean() {
		texture.destroyTextureImage();
		texture.destroy();
		vkDestroySemaphore(device,bindSparseSemaphore,nullptr);
	}
	//根据最小粒度，计算page的数量
	glm::uvec3 alignedDivision(const VkExtent3D& extent, const VkExtent3D& granularity)
	{
		glm::uvec3 res;
		res.x = extent.width / granularity.width + ((extent.width % granularity.width) ? 1u : 0u);// 大于1 是1
		res.y = extent.height / granularity.height + ((extent.height % granularity.height) ? 1u : 0u);
		res.z = extent.depth / granularity.depth + ((extent.depth % granularity.depth) ? 1u : 0u);
		return res;
	}
	void prepareSparseTexture(uint32_t width, uint32_t height, uint32_t layerCount, VkFormat format)
	{
		//大小，格式
		texture.device = device;
		texture.width = width;
		texture.height = height;
		texture.mipLevels = static_cast<uint32_t>(floor(std::log2(std::max(width, height))) + 1);
		texture.mipLevels = 13;
		texture.layerCount = layerCount;
		texture.format = format;

		texture.subRange = { VK_IMAGE_ASPECT_COLOR_BIT,0,texture.mipLevels,0,1 };//baseMip=0, baseLayer=0
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);

		const VkImageType imageType = VK_IMAGE_TYPE_2D;
		const VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;
		const VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;	//拷贝和采样
		const VkImageTiling imageTiling = VK_IMAGE_TILING_OPTIMAL;

		//是否支持, 实际的格式
		std::vector<VkSparseImageFormatProperties> sparseProperties;
		uint32_t sparsePropertiesCount;
		vkGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, imageType, sampleCount, imageUsage, imageTiling, &sparsePropertiesCount, nullptr);
		if (sparsePropertiesCount == 0)
		{
			std::cout << "Error: Requested format does not support sparse features!" << std::endl;
			return;
		}
		sparseProperties.resize(sparsePropertiesCount);
		vkGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, imageType, sampleCount, imageUsage, imageTiling, &sparsePropertiesCount, sparseProperties.data());
		std::cout << "Sparse image format properties: " << sparsePropertiesCount << std::endl;
		for (auto props : sparseProperties)
		{
			std::cout << "\t Image granularity: w = " << props.imageGranularity.width << " h = " << props.imageGranularity.height << " d = " << props.imageGranularity.depth << std::endl;
			std::cout << "\t Aspect mask: " << props.aspectMask << std::endl;
			std::cout << "\t Flags: " << props.flags << std::endl;
		}

		//创建一个 Sparse纹理,flags使用特殊标志位
		VkImageCreateInfo sparseImageCreateInfo{};
		sparseImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		sparseImageCreateInfo.imageType = imageType;
		sparseImageCreateInfo.format = texture.format;
		sparseImageCreateInfo.mipLevels = texture.mipLevels;
		sparseImageCreateInfo.arrayLayers = texture.layerCount;
		sparseImageCreateInfo.samples = sampleCount;
		sparseImageCreateInfo.tiling = imageTiling;
		sparseImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		sparseImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		sparseImageCreateInfo.extent = { texture.width, texture.height, 1 };
		sparseImageCreateInfo.usage = imageUsage;
		sparseImageCreateInfo.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT;
		vkCreateImage(device, &sparseImageCreateInfo, nullptr, &texture.image);
		//未分配内存的情况下，设置 imageLayout
		VkCommandBuffer copyCmd = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
		mg::textures::setImageLayout(copyCmd, texture.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &texture.subRange);
		vulkanDevice->flushCommandBuffer(copyCmd, queue);
		//非虚拟的内存大小 ？
		VkMemoryRequirements sparseImageMemoryReqs;
		vkGetImageMemoryRequirements(device, texture.image, &sparseImageMemoryReqs);

		std::cout << "Image memory requirements:" << std::endl;
		std::cout << "\t Size: " << sparseImageMemoryReqs.size << std::endl;
		std::cout << "\t Alignment: " << sparseImageMemoryReqs.alignment << std::endl;
		//大小超标?
		if (sparseImageMemoryReqs.size > vulkanDevice->properties.limits.sparseAddressSpaceSize)
		{
			std::cout << "Error: Requested sparse image size exceeds supports sparse address space size!" << std::endl;
			return;
		};
		//虚拟纹理的 内存大小?
		uint32_t sparseMemoryReqsCount = 32;
		std::vector<VkSparseImageMemoryRequirements> sparseMemoryReqs(sparseMemoryReqsCount);
		vkGetImageSparseMemoryRequirements(device, texture.image, &sparseMemoryReqsCount, sparseMemoryReqs.data());
		if (sparseMemoryReqsCount == 0)
		{
			std::cout << "Error: No memory requirements for the sparse image!" << std::endl;
			return;
		}
		sparseMemoryReqs.resize(sparseMemoryReqsCount);
		// Get actual requirements
		vkGetImageSparseMemoryRequirements(device, texture.image, &sparseMemoryReqsCount, sparseMemoryReqs.data());

		std::cout << "Sparse image memory requirements: " << sparseMemoryReqsCount << std::endl;
		for (auto reqs : sparseMemoryReqs)
		{
			std::cout << "\t Image granularity: w = " << reqs.formatProperties.imageGranularity.width << " h = " << reqs.formatProperties.imageGranularity.height << " d = " << reqs.formatProperties.imageGranularity.depth << std::endl;
			std::cout << "\t Mip tail first LOD: " << reqs.imageMipTailFirstLod << std::endl;
			std::cout << "\t Mip tail size: " << reqs.imageMipTailSize << std::endl;
			std::cout << "\t Mip tail offset: " << reqs.imageMipTailOffset << std::endl;
			std::cout << "\t Mip tail stride: " << reqs.imageMipTailStride << std::endl;
			//todo:multiple reqs
			texture.mipTailStart = reqs.imageMipTailFirstLod;
		}
		// color aspect ?
		VkSparseImageMemoryRequirements sparseMemoryReq;
		bool colorAspectFound = false;
		for (auto reqs : sparseMemoryReqs)
		{
			if (reqs.formatProperties.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT)
			{
				sparseMemoryReq = reqs;
				colorAspectFound = true;
				break;
			}
		}
		if (!colorAspectFound)
		{
			std::cout << "Error: Could not find sparse image memory requirements for color aspect bit!" << std::endl;
			return;
		}

		// Calculate number of required sparse memory bindings by alignment
		assert((sparseImageMemoryReqs.size % sparseImageMemoryReqs.alignment) == 0);
		texture.memoryTypeIndex = vulkanDevice->getMemoryType(sparseImageMemoryReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		//1366 块?
		uint32_t sparseBindsCount = static_cast<uint32_t>(sparseImageMemoryReqs.size / sparseImageMemoryReqs.alignment);
		std::vector<VkSparseMemoryBind>	sparseMemoryBinds(sparseBindsCount);

		texture.sparseImageMemoryRequirements = sparseMemoryReq;

		// The mip tail contains all mip levels > sparseMemoryReq.imageMipTailFirstLod
		// Check if the format has a single mip tail for all layers or one mip tail for each layer
		texture.mipTailInfo.singleMipTail = sparseMemoryReq.formatProperties.flags & VK_SPARSE_IMAGE_FORMAT_SINGLE_MIPTAIL_BIT;
		texture.mipTailInfo.alingedMipSize = sparseMemoryReq.formatProperties.flags & VK_SPARSE_IMAGE_FORMAT_ALIGNED_MIP_SIZE_BIT;

		//除了 mip-tail中的小图，其它的绑定为 page
		for (uint32_t layer = 0; layer < texture.layerCount; layer++)
		{
			//将一个 Mip等级(小于mip-tail) 划分为Page,并记录 Offset,Layer,Mip-Level, 以及根据添加顺序得到的Index
			for (uint32_t mipLevel = 0; mipLevel < sparseMemoryReq.imageMipTailFirstLod; mipLevel++)
			{
				VkExtent3D extent;//一个 mip等级的大小
				extent.width = std::max(sparseImageCreateInfo.extent.width >> mipLevel, 1u);
				extent.height = std::max(sparseImageCreateInfo.extent.height >> mipLevel, 1u);
				extent.depth = std::max(sparseImageCreateInfo.extent.depth >> mipLevel, 1u);

				VkImageSubresource subResource{};
				subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				subResource.mipLevel = mipLevel;
				subResource.arrayLayer = layer;

				VkExtent3D imageGranularity = sparseMemoryReq.formatProperties.imageGranularity;
				glm::uvec3 sparseBindCounts = alignedDivision(extent, imageGranularity);//根据粒度，计算绑定的数量
				glm::uvec3 lastBlockExtent;
				//只有小于粒度128时，才会使用 64,32等等 (逻辑走不到这块)
				lastBlockExtent.x = (extent.width % imageGranularity.width) ? extent.width % imageGranularity.width : imageGranularity.width;
				lastBlockExtent.y = (extent.height % imageGranularity.height) ? extent.height % imageGranularity.height : imageGranularity.height;
				lastBlockExtent.z = (extent.depth % imageGranularity.depth) ? extent.depth % imageGranularity.depth : imageGranularity.depth;

				uint32_t index = 0;
				for (uint32_t z = 0; z < sparseBindCounts.z; z++)
				{
					for (uint32_t y = 0; y < sparseBindCounts.y; y++)
					{
						for (uint32_t x = 0; x < sparseBindCounts.x; x++)//对每一个Mip等级进行 最小粒度128大小的划分
						{
							// Offset
							VkOffset3D offset;
							offset.x = x * imageGranularity.width;
							offset.y = y * imageGranularity.height;
							offset.z = z * imageGranularity.depth;
							// Size of the page
							VkExtent3D extent;
							extent.width = (x == sparseBindCounts.x - 1) ? lastBlockExtent.x : imageGranularity.width;
							extent.height = (y == sparseBindCounts.y - 1) ? lastBlockExtent.y : imageGranularity.height;
							extent.depth = (z == sparseBindCounts.z - 1) ? lastBlockExtent.z : imageGranularity.depth;

							// Add new virtual page
							VirtualTexturePage* newPage = texture.addPage(offset, extent, sparseImageMemoryReqs.alignment, mipLevel, layer);
							newPage->imageMemoryBind.subresource = subResource;

							index++;
						}
					}
				}//将一个 Mip等级 划分为多个 Page
			}//一个Mip等级的 划分
			//每层余下的 mip-tail 会被放到 opaque 内存
			if ((!texture.mipTailInfo.singleMipTail) && (sparseMemoryReq.imageMipTailFirstLod < texture.mipLevels))
			{
				// Allocate memory for the mip tail
				VkMemoryAllocateInfo allocInfo{};
				allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				allocInfo.allocationSize = sparseMemoryReq.imageMipTailSize;
				allocInfo.memoryTypeIndex = texture.memoryTypeIndex;

				VkDeviceMemory deviceMemory;
				vkAllocateMemory(device, &allocInfo, nullptr, &deviceMemory);

				// (Opaque) sparse memory binding
				VkSparseMemoryBind sparseMemoryBind{};
				sparseMemoryBind.resourceOffset = sparseMemoryReq.imageMipTailOffset + layer * sparseMemoryReq.imageMipTailStride;
				sparseMemoryBind.size = sparseMemoryReq.imageMipTailSize;
				sparseMemoryBind.memory = deviceMemory;

				texture.opaqueMemoryBinds.push_back(sparseMemoryBind);
			}
		}//一个Layer

		std::cout << "Texture info:" << std::endl;
		std::cout << "\tDim: " << texture.width << " x " << texture.height << std::endl;
		std::cout << "\tVirtual pages: " << texture.pages.size() << std::endl;

		// Check if format has one mip tail for all layers
		if ((sparseMemoryReq.formatProperties.flags & VK_SPARSE_IMAGE_FORMAT_SINGLE_MIPTAIL_BIT) && (sparseMemoryReq.imageMipTailFirstLod < texture.mipLevels))
		{
			// Allocate memory for the mip tail
			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = sparseMemoryReq.imageMipTailSize;
			allocInfo.memoryTypeIndex = texture.memoryTypeIndex;

			VkDeviceMemory deviceMemory;
			vkAllocateMemory(device, &allocInfo, nullptr, &deviceMemory);

			// (Opaque) sparse memory binding
			VkSparseMemoryBind sparseMemoryBind{};
			sparseMemoryBind.resourceOffset = sparseMemoryReq.imageMipTailOffset;
			sparseMemoryBind.size = sparseMemoryReq.imageMipTailSize;
			sparseMemoryBind.memory = deviceMemory;

			texture.opaqueMemoryBinds.push_back(sparseMemoryBind);
		}

		VkSemaphoreCreateInfo semaphoreCreateInfo{};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &bindSparseSemaphore);
		//预备 page的绑定信息
		texture.updateSparseBindInfo(texture.pages);
		vkQueueBindSparse(queue, 1, &texture.bindSparseInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(queue);

		//创建Sampler
		VkSamplerCreateInfo sampler{};
		sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler.maxAnisotropy = 1.0f;
		sampler.magFilter = VK_FILTER_LINEAR;
		sampler.minFilter = VK_FILTER_LINEAR;
		sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler.mipLodBias = 0.0f;
		sampler.compareOp = VK_COMPARE_OP_NEVER;
		sampler.minLod = 0.0f;
		sampler.maxLod = static_cast<float>(texture.mipLevels);
		sampler.maxAnisotropy = vulkanDevice->features.samplerAnisotropy ? vulkanDevice->properties.limits.maxSamplerAnisotropy : 1.0f;
		sampler.anisotropyEnable = false;
		sampler.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		vkCreateSampler(device, &sampler, nullptr, &texture.sampler);

		// Create image view
		VkImageViewCreateInfo view{};
		view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view.image = VK_NULL_HANDLE;
		view.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view.format = format;
		view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		view.subresourceRange.baseMipLevel = 0;
		view.subresourceRange.baseArrayLayer = 0;
		view.subresourceRange.layerCount = 1;
		view.subresourceRange.levelCount = texture.mipLevels;
		view.image = texture.image;
		vkCreateImageView(device, &view, nullptr, &texture.view);

		// Fill image descriptor image info that can be used during the descriptor set setup
		texture.descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		texture.descriptor.imageView = texture.view;
		texture.descriptor.sampler = texture.sampler;
	}
};
