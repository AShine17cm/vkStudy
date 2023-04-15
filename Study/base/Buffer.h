#pragma once

#include<vulkan/vulkan.h>

namespace mg
{
	/*
	对 Uniform Buffer Object 的封装，
	包括VkBuffer，内存，DescriptorInfo
	常用操作，copy,flush
	*/
	struct Buffer
	{
		VkDevice device;
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkDescriptorBufferInfo descriptor;			//如果DescriptorType为dynamic, range需设置为单个stride,而不是buffer的整体大小

		VkDeviceSize size = 0;
		VkDeviceSize alignment = 0;
		void* mapped = nullptr;

		VkBufferUsageFlags usage;
		VkMemoryPropertyFlags memoryProperties;

		VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void unmap();
		VkResult bind(VkDeviceSize offset = 0);
		void setupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void copyTo(void* data, VkDeviceSize size);
		VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void destroy();
	};

}
