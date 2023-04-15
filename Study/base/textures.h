#pragma once
#include "vulkan/vulkan.h"
#include "VulkanDevice.h"



namespace mg
{
namespace textures
{
		enum  class MgTextureEx
		{
			None = 0,
			//Image
			Image_None = 100,
			Image_Cube = 101,

			//Sampler
			Sampler_None = 300,
			Sampler_AnisotropyDisable = 301,
			Sampler_BorderWhite = 302,
		};
		struct MgImageInfo
		{
			VkExtent3D extent3D = { 0,0,1 };
			uint32_t layers = 1;
			uint32_t mipLevels = 1;
			struct Formats
			{
				VkImageType type = VK_IMAGE_TYPE_2D;
				VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
				VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
				VkImageLayout imagelayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			}formats;
		};
		//可处理Array, 内存布局和ktx相关
		void createTexture(
			MgImageInfo info,
			uint32_t size,
			VulkanDevice* device,
			VkImage* image,
			VkDeviceMemory* imageMemory,
			void* texData,
			uint32_t* offsets,
			MgTextureEx* extend,
			uint32_t exCount);
		/*
		不使用stage-memory-host
		layers-miplevel 为 1
		*/
		void createTexture_Simple(
			MgImageInfo imgInfo,
			void* texData,
			VulkanDevice* device,
			VkImage* image,
			VkDeviceMemory* imageMemory,
			MgTextureEx* extFeatures=nullptr,
			uint32_t extCount = 0);


		void createImageView(
			VkDevice device,
			VkImage image,
			VkImageView* view,
			MgImageInfo imgInfo,
			VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D);

		void createSampler(
			VulkanDevice* device,
			uint32_t mipLevels,
			VkSampler* sampler,
			MgTextureEx* extends,
			uint32_t extendCount,
			VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);

		/*
		从image-Layout 确定 内存access类型
		*/
		void setImageLayout(
			VkCommandBuffer cmdBuffer,
			VkImage image,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkImageSubresourceRange* subresourceRange,
			VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

}
}

