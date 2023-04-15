#include "buffers.h"

namespace mg
{
	namespace buffers
	{
		//将数据拷贝到 device local内存中
		void transferDataLocal(
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VulkanDevice* vulkanDevice,
			VkQueue copyQueue,
			Buffer* mgBuffer,
			void* data)
		{
			//
			VkDevice logicalDevice = vulkanDevice->logicalDevice;
			VkMemoryPropertyFlags memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			struct
			{
				VkBuffer buffer;
				VkDeviceMemory memory;
			}stagingBuffer;

			vulkanDevice->createBuffer(
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				size,
				&stagingBuffer.buffer,
				&stagingBuffer.memory,
				data);
			//创建 mgBuffer
			vulkanDevice->createBuffer(
				usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				memProperties,
				size,
				mgBuffer);
			//
			VkCommandBuffer cmdCopy = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
			VkBufferCopy copy{ 0,0, size };
			vkCmdCopyBuffer(
				cmdCopy,
				stagingBuffer.buffer,
				mgBuffer->buffer,
				1, &copy);

			vulkanDevice->flushCommandBuffer(cmdCopy, copyQueue, true);
			vkDestroyBuffer(logicalDevice, stagingBuffer.buffer, nullptr);
			vkFreeMemory(logicalDevice, stagingBuffer.memory, nullptr);
		}
		void transferDataLocal(
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VulkanDevice* vulkanDevice,
			VkQueue copyQueue,
			VkBuffer* localBuffer,
			VkDeviceMemory* localMemory,
			void* data)
		{
			//
			VkDevice logicalDevice = vulkanDevice->logicalDevice;
			VkMemoryPropertyFlags memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			struct
			{
				VkBuffer buffer;
				VkDeviceMemory memory;
			}stagingBuffer;

			vulkanDevice->createBuffer(
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				size,
				&stagingBuffer.buffer,
				&stagingBuffer.memory,
				data);

			vulkanDevice->createBuffer(
				usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				memProperties,
				size,
				localBuffer,
				localMemory);

			VkCommandBuffer cmdCopy = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
			VkBufferCopy copy{ 0,0, size };
			vkCmdCopyBuffer(
				cmdCopy,
				stagingBuffer.buffer,
				*localBuffer,
				1, &copy);

			vulkanDevice->flushCommandBuffer(cmdCopy, copyQueue, true);
			vkDestroyBuffer(logicalDevice, stagingBuffer.buffer, nullptr);
			vkFreeMemory(logicalDevice, stagingBuffer.memory, nullptr);
		}


	}
}