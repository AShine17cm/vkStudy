#pragma once
#include "textures.h"
namespace mg 
{
namespace mipmaps 
{
	void generateMipmaps(
		uint32_t baseLayer,
		uint32_t mipLevels,
		uint32_t width,
		uint32_t height,
		VkImage srcImage,
		VkImage dstImage,
		VkCommandBuffer blitCmd
	);

	void generateMipmaps(
		textures::MgImageInfo imgInfo,
		VkImage image,
		VulkanDevice* vulkanDevice,
		VkQueue queue
	);
}
}
