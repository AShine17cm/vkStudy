#pragma once
#include <vector>
#include "glm.hpp"
#include "vulkan/vulkan.h"
#include "textures.h"
#include "Texture.h"
#include "SimpleRandom.h"
#include <chrono>
using namespace mg;
/* 把Texture又包装了一层 */
struct Tex3D
{
	uint32_t width = 128;
	uint32_t height = 128;
	uint32_t depth = 128;
	VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;//255
	textures::Texture* texture;

	/* 创建没有数据的对象 */
	Tex3D(mg::VulkanDevice* vulkanDevice, uint32_t width, uint32_t height, uint32_t depth)
	{
		this->width = width;
		this->height = height;
		this->depth = depth;
		textures::MgImageInfo info{};
		info.extent3D = { width,height,depth };
		info.formats.format = format;
		info.formats.type = VK_IMAGE_TYPE_3D;

		uint32_t channel = 4;
		texture = new textures::Texture(vulkanDevice, info);
		texture->viewType = VK_IMAGE_VIEW_TYPE_3D;
		texture->load(nullptr, channel);
	}
	/* 生成数据 */
	void generate()
	{
		uint32_t channel = 4;
		const uint32_t memSize = width * height * depth * channel;
		uint8_t* data = new uint8_t[memSize];
		memset(data, 0, memSize);
		for (uint8_t d = 0; d < depth; d++)
		{
			float nd = (float)d / (float)depth;
			for (uint8_t y = 0; y < height; y++)
			{
				float ny = (float)y / (float)height;
				for (uint8_t x = 0; x < width; x++)
				{
					float nx = (float)x / (float)width;
					//float tmp = rnd.generate() * memSize;
					uint32_t offset = x + y * width + d * width * height;
					data[offset * 4 + 0] = static_cast<uint8_t>(floor(nx * 255));
					data[offset * 4 + 1] = static_cast<uint8_t>(floor(ny * 255));
					data[offset * 4 + 2] = static_cast<uint8_t>(floor(nd * 255));
					data[offset * 4 + 3] = 255;
				}
			}
		}
		VkDeviceSize imageSize = memSize;
		textures::MgInsertPiece piece = { texture->info,0,0,imageSize };
		textures::InsertPiece(texture->vulkanDevice, &texture->image, data, piece);

		delete[] data;
	}
	void clean()
	{
		texture->destroy();
	}
};