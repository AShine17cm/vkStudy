#include "Texture.h"
#include "mipmaps.h"
//#include "textures.h"
#include <algorithm>

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
	void Texture::load(const char* filename,int channels) 
	{
		//第三方库相关
		void* data = nullptr;
		if (nullptr != filename) 
		{
			int width = 0, height = 0;
			//channels = 0;
			data=stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);
			info.extent3D.width = width;
			info.extent3D.height = height;
			//需要生成 mipmaps
			if (info.gen_Mips) 
			{
				int maxBorder = std::max(width, height);
				int numLevels = 1 + floor(log(maxBorder));
				info.mipLevels = numLevels;
			}
		}
		if (VK_FORMAT_R8G8B8A8_SRGB == info.formats.format)channels = 4;//简单处理
		if (info.gen_Mips)
		{
			info.formats.usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT; //最后再去掉 ?
		}

		VkDeviceSize imageSize = info.extent3D.width * info.extent3D.height *info.extent3D.depth* channels;
		textures::MgImgViewInfo viewInfo{};
		viewInfo.layerCount = info.layers;
		viewInfo.mipLevCount = info.mipLevels;
		viewInfo.imgFormat = info.formats.format;
		viewInfo.viewType = viewType;
		//数据格式 相关
		textures::createTexture(info,imageSize,vulkanDevice,&image, &memory,data,nullptr,0);
		textures::createImageView(vulkanDevice->logicalDevice,image, &view,viewInfo);
		textures::createSampler(vulkanDevice,info.mipLevels,&sampler,extends.data(), extends.size(),info.formats.samplerMode);
		updateDescriptor();
	}
	/* 后续操作，必须比创建时的 <width,height>区域 =/小 */
	void Texture::Insert(const char* filename,uint32_t target_Layer,uint32_t target_mipLevel) 
	{
		void* data = nullptr;
		int width = 0, height = 0, channels = 0;
		data = stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);

		VkDeviceSize imageSize = width * height * info.extent3D.depth * 4;
		MgInsertPiece piece = { info,target_Layer,target_mipLevel,imageSize};
		textures::InsertPiece(vulkanDevice, &image,data, piece);
	}
	void Texture::genMips()
	{
		if (info.gen_Mips)
		{
			mipmaps::generateMipmaps(
				info,
				image,
				vulkanDevice,
				vulkanDevice->graphicsQueue);
		}
	}
	void Texture::updateDescriptor()
	{
		descriptor.sampler = sampler;
		descriptor.imageView = view;
		descriptor.imageLayout =info.formats.imagelayout;
	}
	void Texture::overrideDescriptor(VkImageLayout imgLayout)//延迟渲染  Compose阶段 不能使用Image创建时的Layout
	{
		descriptor.sampler = sampler;
		descriptor.imageView = view;
		descriptor.imageLayout = imgLayout;
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