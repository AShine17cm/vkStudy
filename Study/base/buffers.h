#pragma once
#include <vulkan/vulkan.h>
#include "VulkanDevice.h"
#include "VulkanTools.h"
namespace mg
{
	namespace buffers
	{
		//�����ݿ����� device local�ڴ���
		/*
		ʹ�÷���� VkBuffer���ڴ�
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
		ʹ�÷�װ��Buffer
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

