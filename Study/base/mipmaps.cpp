#include "mipmaps.h"

namespace mg 
{
namespace mipmaps 
{
	void generateMipmaps(
		uint32_t layerCount,
		uint32_t mipLevels,
		uint32_t width,
		uint32_t height,
		VkImage srcImage,
		VkImage dstImage,
		VkCommandBuffer blitCmd)
	{
		//Copy down mips from n - 1 to n
		for (uint32_t i = 1; i < mipLevels; i++) {

			VkImageBlit imageBlit{};
			//Source
			imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlit.srcSubresource.baseArrayLayer = 0;
			imageBlit.srcSubresource.layerCount = layerCount;
			imageBlit.srcSubresource.mipLevel = i - 1;
			imageBlit.srcOffsets[1].x = width >> (i - 1);
			imageBlit.srcOffsets[1].y = height >> (i - 1);
			imageBlit.srcOffsets[1].z = 1;

			//Destination
			imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlit.dstSubresource.baseArrayLayer = 0;
			imageBlit.dstSubresource.layerCount = layerCount;
			imageBlit.dstSubresource.mipLevel = i;
			imageBlit.dstOffsets[1].x = width >> i;
			imageBlit.dstOffsets[1].y = height >> i;
			imageBlit.dstOffsets[1].z = 1;

			VkImageSubresourceRange mipSubRange;
			mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			mipSubRange.baseMipLevel = i;
			mipSubRange.baseArrayLayer = 0;
			mipSubRange.layerCount = layerCount;
			mipSubRange.levelCount = 1;

			// Prepare current mip level as image blit destination
			textures::setImageLayout(
				blitCmd,
				srcImage,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				&mipSubRange,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT);

			vkCmdBlitImage(
				blitCmd,
				srcImage,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				dstImage,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&imageBlit,
				VK_FILTER_LINEAR);

			//Prepare current mip level as image blit source for next level
			textures::setImageLayout(
				blitCmd,
				dstImage,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				&mipSubRange,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT);
		}
	}
	void generateMipmaps(
		textures::MgImageInfo imgInfo,
		VkImage image,
		VulkanDevice* vulkanDevice,
		VkQueue queue)
	{
		VkCommandBuffer blitCmd = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
		//将第一层 从 SHADER_READ 转换到 TRANSFER_SRC
		VkImageSubresourceRange range = { VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,imgInfo.layers };
		textures::setImageLayout(
			blitCmd,
			image,
			imgInfo.formats.imagelayout,		//VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			&range,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

		//逐层生成 mips
		generateMipmaps(imgInfo.layers,
			imgInfo.mipLevels,
			imgInfo.extent3D.width,
			imgInfo.extent3D.height,
			image, image, blitCmd);

		VkImageSubresourceRange subresourceRange{};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;			
		subresourceRange.levelCount = imgInfo.mipLevels;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = imgInfo.layers;

		//将所有的 TRANSFER_SRC 转换到 SHADER_READ
		textures::setImageLayout(
			blitCmd,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			imgInfo.formats.imagelayout,		//VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			&subresourceRange,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

		vulkanDevice->flushCommandBuffer(blitCmd, queue, true);
	}
}
}