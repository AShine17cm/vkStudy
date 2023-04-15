#pragma once
#include "mg.h"

namespace mg
{
	/*
	RenderPass-复用,材质-复用
	角度不同，FrameBuffer大小不同
	*/
	struct OffscreenPass
	{
		VkExtent2D extent;
		VkFramebuffer frameBuffer;
		textures::Texture *color, *depth;
		void prepare(VulkanDevice* vulkanDevice, VkFormat format, VkFormat depthFormat,VkRenderPass renderPass);
		void cleanup(VkDevice device);
	};
}
