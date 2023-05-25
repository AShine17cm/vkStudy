#pragma once
#include <vulkan/vulkan.h>
#include <assert.h>
#include <exception>
#include <vector>
#include <string>

#include "Buffer.h"
namespace mg
{

	struct VulkanDevice
	{
		VkPhysicalDevice physicalDevice;
		VkDevice logicalDevice;
		VkCommandPool commandPool = VK_NULL_HANDLE;
		VkQueue graphicsQueue;	//VkQueue随 VkDevice存在 不用单独销毁
		VkQueue presentQueue;

		VkPhysicalDeviceProperties properties;
		VkPhysicalDeviceFeatures features;
		VkPhysicalDeviceFeatures enabledFeatures;

		VkPhysicalDeviceMemoryProperties memoryProperties;
		std::vector<VkQueueFamilyProperties> queueFamilyProperties;
		std::vector<std::string> supportedExtensions;

		bool enableDebugMarkers = false;

		struct
		{
			uint32_t graphics;
			uint32_t compute;
			uint32_t transfer;
			//uint32_t sparse;
			//uint32_t protected;
			uint32_t present;	//使用graphics
		}queueFamilyIndices;

		operator VkDevice() const {
			return logicalDevice;
		}
		explicit VulkanDevice(VkPhysicalDevice physicalDevice);
		~VulkanDevice();
		//1.内存类型
		//2.Queue家族
		//3.LogicalDevice
		uint32_t		getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound = nullptr)const;
		uint32_t		getQueueFamilyIndex(VkQueueFlags queueFlags)const;
		VkResult		createLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures, std::vector<const char*> extensions, void* nextChain,
						VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT);
		//1.分别传出 VkBuffer和VkDeviceMemory
		//2.使用封装的 mg::Buffer
		VkResult		createBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperties, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* deviceMemory, void* data = nullptr);
		VkResult		createBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperties, VkDeviceSize size, Buffer* buffer, void* data = nullptr);
		void			copyBuffer(Buffer* src, Buffer* dst, VkQueue queue, VkBufferCopy* copyRegion = nullptr);
		//Command相关
		VkCommandPool	createCommandPool(uint32_t queueFamilyIndex,
						VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		void			createCommandBuffers(VkCommandBufferLevel level,VkCommandBuffer* cmdBuffers, uint32_t count);
		VkCommandBuffer	createCommandBuffer(VkCommandBufferLevel level,bool begin=false);
		void			beginCommandBuffer(VkCommandBuffer commandBuffer);
		void			flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true);
		//
		bool			extensionSupported(std::string extension);
		VkFormat		getSupportedDepthFormat(bool checkSamplingSupport);

	};

}
