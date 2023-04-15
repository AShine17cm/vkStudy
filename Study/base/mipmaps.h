#pragma once
#include "textures.h"
namespace mg 
{
namespace mipmaps 
{
	void generateMipmaps(
		uint32_t width,
		uint32_t height,
		//uint32_t depth,
		uint32_t mipLevels,
		VkImage srcImage,
		VkImage dstImage,
		VkCommandBuffer blitCmd
	);

	void generateMipmaps(
		uint32_t width,
		uint32_t height,
		//uint32_t depth,
		uint32_t mipLevels,
		VkImage image,
		VulkanDevice* vulkanDevice,
		VkQueue queue
	);
}
}
