#pragma once
#include <vulkan/vulkan.h>
#include "VulkanDevice.h"
#include "VulkanTools.h"
namespace mg
{
	namespace buffers
	{
		//将数据拷贝到 device local内存中
		/*
		使用分离的 VkBuffer和内存
		*/
		void transferDataLocal(
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VulkanDevice* vulkanDevice,
			VkQueue copyQueue,
			VkBuffer* localBuffer,
			VkDeviceMemory* localMemory,
			void* data);
		/*
		使用封装的Buffer
		*/
		void transferDataLocal(
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VulkanDevice* vulkanDevice,
			VkQueue copyQueue,
			Buffer* mgBuffer,
			void* data);
	}
}

