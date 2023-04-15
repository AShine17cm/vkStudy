#pragma once
#include "VulkanDevice.h"
#include "textures.h"
namespace mg 
{
namespace textures 
{
	/*
	数据加载，创建
	image,view,memory,sampler,descriptor
	*/
	class  Texture
	{
	public:
		VulkanDevice* vulkanDevice;
		MgImageInfo info;
		VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;

		VkImage image;
		VkImageView view;
		VkDeviceMemory memory;

		VkDescriptorImageInfo descriptor;
		VkSampler sampler;

		Texture(VulkanDevice* vulkanDevice,textures::MgImageInfo extent);
		void updateDescriptor();
		void destroy();
		void load(const char* filename);

	private:

	};

}
}
