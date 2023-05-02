#pragma once
#include "Texture.h"
#include "vulkan/vulkan.h"
using namespace mg;

/* 一个Image的部分层,MipLevel */
struct  SubImageView
{
	VkImageView imgView;
	VkSampler sampler;
	VkDescriptorImageInfo descriptor;
	textures::MgImgViewInfo viewInfo;
	
	SubImageView(textures::Texture* tex,textures::MgImgViewInfo viewInfo)
	{
		this->viewInfo = viewInfo;

		textures::createImageView(
			tex->vulkanDevice->logicalDevice,
			tex->image, 
			&imgView,
			viewInfo);

		textures::createSampler(
			tex->vulkanDevice, 
			viewInfo.mipLevCount,
			&sampler,
			nullptr, 0);

		descriptor.sampler = sampler;
		descriptor.imageView = imgView;
		descriptor.imageLayout = tex->info.formats.imagelayout;
	}
	void clean(VkDevice device)
	{
		vkDestroyImageView(device, imgView, nullptr);
		if (sampler)
		{
			vkDestroySampler(device, sampler, nullptr);
		}
	}
};