#pragma once
#include "mg.h"

namespace mg
{
	/*
	RenderPass-����,����-����
	�ǶȲ�ͬ��FrameBuffer��С��ͬ
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
