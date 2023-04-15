#include "Texture.h"
//#include "textures.h"

//第三方库
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
//第三方库
namespace mg 
{
namespace textures 
{
	Texture::Texture(VulkanDevice* vulkanDevice, MgImageInfo imgInfo)
	{
		this->vulkanDevice = vulkanDevice;
		this->info = imgInfo;
	}
	void Texture::load(const char* filename) 
	{
		//第三方库相关
		void* data = nullptr;
		if (nullptr != filename) 
		{
			int width=0, height=0, channels=0;
			data=stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);
			info.extent3D.width = width;
			info.extent3D.height = height;
		}

		VkDeviceSize imageSize = info.extent3D.width * info.extent3D.height *info.extent3D.depth* 4;

		std::vector<uint32_t> offsets = { 0 };
		//数据格式 相关
		//textures::createTexture_Simple(info,data,vulkanDevice,&image,&memory);
		textures::createTexture(info,imageSize,vulkanDevice,&image, &memory,data,offsets.data(),nullptr,0);
		textures::createImageView(vulkanDevice->logicalDevice,image, &view,info,viewType);
		textures::createSampler(vulkanDevice,info.mipLevels,&sampler,nullptr, 0,VK_SAMPLER_ADDRESS_MODE_REPEAT);

		updateDescriptor();
	}
	void Texture::updateDescriptor()
	{
		descriptor.sampler = sampler;
		descriptor.imageView = view;
		descriptor.imageLayout =info.formats.imagelayout;
	}
	void Texture::destroy()
	{
		vkDestroyImageView(vulkanDevice->logicalDevice, view, nullptr);
		vkDestroyImage(vulkanDevice->logicalDevice, image, nullptr);
		if (sampler)
		{
			vkDestroySampler(vulkanDevice->logicalDevice, sampler, nullptr);
		}
		vkFreeMemory(vulkanDevice->logicalDevice, memory, nullptr);
	}
}
}