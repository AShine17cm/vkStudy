#include "mipmaps.h"

namespace mg 
{
namespace mipmaps 
{
	void generateMipmaps(
		uint32_t width,
		uint32_t height,
		uint32_t mipLevels,
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
			imageBlit.srcSubresource.layerCount = 1;
			imageBlit.srcSubresource.mipLevel = i - 1;
			imageBlit.srcOffsets[1].x = width >> (i - 1);
			imageBlit.srcOffsets[1].y = height >> (i - 1);
			imageBlit.srcOffsets[1].z = 1;

			//Destination
			imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlit.dstSubresource.baseArrayLayer = 0;
			imageBlit.dstSubresource.layerCount = 1;
			imageBlit.dstSubresource.mipLevel = i;
			imageBlit.dstOffsets[1].x = width >> i;
			imageBlit.dstOffsets[1].y = height >> i;
			imageBlit.dstOffsets[1].z = 1;

			VkImageSubresourceRange mipSubRange;
			mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			mipSubRange.baseMipLevel = i;
			mipSubRange.layerCount = 1;
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
		uint32_t width,
		uint32_t height,
		uint32_t mipLevels,
		VkImage image,
		VulkanDevice* vulkanDevice,
		VkQueue queue)
	{
		VkCommandBuffer blitCmd = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
		// Generate the mip chain
		// ---------------------------------------------------------------
		// We copy down the whole mip chain doing a blit from mip-1 to mip
		// An alternative way would be to always blit from the first mip level and sample that one down
		generateMipmaps(width, height, mipLevels, image, image, blitCmd);
		//
		VkImageSubresourceRange subresourceRange{};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.levelCount = mipLevels;
		subresourceRange.layerCount = 1;

		//After the loop, all mip layers are in TRANSFER_SRC layout
		//so transition all to SHADER_READ
		textures::setImageLayout(
			blitCmd,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			&subresourceRange,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

		vulkanDevice->flushCommandBuffer(blitCmd, queue, true);
	}
}
}