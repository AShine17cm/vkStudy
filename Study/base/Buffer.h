#pragma once

#include<vulkan/vulkan.h>

namespace mg
{
	/*
	�� Uniform Buffer Object �ķ�װ��
	����VkBuffer���ڴ棬DescriptorInfo
	���ò�����copy,flush
	*/
	struct Buffer
	{
		VkDevice device;
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkDescriptorBufferInfo descriptor;			//���DescriptorTypeΪdynamic, range������Ϊ����stride,������buffer�������С

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
