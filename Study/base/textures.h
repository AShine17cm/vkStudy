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
			Image_Layer=102,
			Image_Array=103,
			Image_Mipmap=104,
			Image_Depth=105,

			MULTI_SAMPLE=200,

			//Sampler
			Sampler_None = 300,
			Sampler_AnisotropyDisable = 301,
			Sampler_BorderWhite = 302,
			Sampler_ShadowMap=303,

			NO_ExtraSetting=400,	//不做额外的设置
		};
		struct MgImageInfo
		{
			VkExtent3D extent3D = { 0,0,1 };
			uint32_t layers = 1;
			uint32_t mipLevels = 1;		
			bool gen_Mips = false;
			struct Formats
			{
				VkImageType type = VK_IMAGE_TYPE_2D;
				VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
				VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
				VkImageLayout imagelayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;		//用于 descriptor 的创建
				VkImageCreateFlags createFalgs = 0;// VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
				VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;
				VkSamplerAddressMode samplerMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			}formats;

			VkExtent2D getExtend2D() 
			{
				VkExtent2D ex2d = { extent3D.width,extent3D.height };
				return ex2d;
			}
		};
		struct  MgImgViewInfo		//image资源的一部分
		{
			uint32_t baseLayer = 0;
			uint32_t baseMipLevel = 0;
			uint32_t layerCount = 1;
			uint32_t mipLevCount = 1;
			VkFormat imgFormat = VK_FORMAT_R8G8B8A8_SRGB;
			VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
			
		};
		struct MgInsertPiece		//用于插入 一块数据<layer，mip-level>
		{
			VkExtent3D imgExtend3D;

			uint32_t target_Layer = -1;
			uint32_t target_MipLevel = -1;

			VkDeviceSize dataSize = 0;
			VkImageLayout img_Layout;

			MgInsertPiece(MgImageInfo imgInfo,uint32_t target_Layer,uint32_t target_MipLevel,VkDeviceSize dataSize) 
			{
				imgExtend3D = imgInfo.extent3D;
				img_Layout = imgInfo.formats.imagelayout;

				this->target_Layer = target_Layer;
				this->target_MipLevel = target_MipLevel;
				this->dataSize = dataSize;
			}
			VkBufferImageCopy  GetCopy() 
			{
				uint32_t layerCount = 1;		//只操作一层
				VkBufferImageCopy  copy{};
				copy.imageOffset = { 0,0,0 };	//数据块从0开始
				copy.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT,target_MipLevel,target_Layer,layerCount };
				/* piece size */
				copy.imageExtent = { imgExtend3D.width >> target_MipLevel,imgExtend3D.height >> target_MipLevel,imgExtend3D.depth};
				copy.bufferOffset = 0;
				return copy;
			}
			/* 操作时 设置最佳的 image-layout */
			VkImageSubresourceRange GetLayoutRange() {
				uint32_t operate_Count = 1;
				uint32_t operate_MipLevCount = 1;
				VkImageSubresourceRange subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT,target_MipLevel,operate_MipLevCount,target_Layer,operate_Count };
				return subresourceRange;
			}
		};
		//可处理Array, 内存布局和ktx相关
		void createTexture(
			MgImageInfo info,
			uint32_t insertSize,
			VulkanDevice* device,
			VkImage* image,
			VkDeviceMemory* imageMemory,
			void* texData,
			MgTextureEx* extend,
			uint32_t exCount);
		/* 在某一层，某一个mipLevel中插入数据 */
		void InsertPiece(VulkanDevice* device,VkImage *image,void* texData,MgInsertPiece piece);
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
			MgImgViewInfo viewInfo);

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

