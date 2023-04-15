//#include "pch.h"
#include <stdexcept>
#include <iostream>
#include "VulkanDevice.h"
#include "VulkanTools.h"
#include "Buffer.h"
namespace mg
{
	/*
	显卡 属性-Feature-内存-queue-extension
	*/
	VulkanDevice::VulkanDevice(VkPhysicalDevice physicalDevice) {
		assert(physicalDevice);
		this->physicalDevice = physicalDevice;

		vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		vkGetPhysicalDeviceFeatures(physicalDevice, &features);
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
		assert(queueFamilyCount > 0);
		queueFamilyProperties.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

		uint32_t extCount;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extCount);
		MG_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, &extensions.front()));
		for (const auto& ext : extensions)
		{
			supportedExtensions.push_back(ext.extensionName);
		}


	}
	VulkanDevice::~VulkanDevice() {
		if (commandPool) {
			vkDestroyCommandPool(logicalDevice, commandPool, nullptr);
		}
		if (logicalDevice) {
			vkDestroyDevice(logicalDevice, nullptr);
		}
	}
	uint32_t VulkanDevice::getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound)const {

		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
			if ((1 << i & typeBits) != 0)
			{
				if ((properties & memoryProperties.memoryTypes[i].propertyFlags) == properties)
				{
					if (memTypeFound)
					{
						*memTypeFound = true;
					}
					return i;
				}
			}
		}
		if (memTypeFound) {
			*memTypeFound = false;
			return 0;
		}
		else
		{
			throw std::runtime_error("Could not find a matching memory type");
		}
	}
	/*
	最优专用的 queue
	*/
	uint32_t VulkanDevice::getQueueFamilyIndex(VkQueueFlags queueFlags)const {

		// Dedicated queue for compute
		// Try to find a queue family index that supports compute but not graphics
		if ((VK_QUEUE_COMPUTE_BIT & queueFlags) == queueFlags) {
			for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
			{
				if ((VK_QUEUE_COMPUTE_BIT & queueFamilyProperties[i].queueFlags) &&
					(VK_QUEUE_GRAPHICS_BIT & queueFamilyProperties[i].queueFlags) == 0) {
					return i;
				}
			}
		}

		// Dedicated queue for transfer
		// Try to find a queue family index that supports transfer but not graphics and compute
		if ((VK_QUEUE_TRANSFER_BIT & queueFlags) == queueFlags) {
			for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
			{
				if ((VK_QUEUE_COMPUTE_BIT & queueFamilyProperties[i].queueFlags) &&
					(VK_QUEUE_GRAPHICS_BIT & queueFamilyProperties[i].queueFlags) == 0 &&
					(VK_QUEUE_COMPUTE_BIT & queueFamilyProperties[i].queueFlags) == 0) {
					return i;
				}
			}
		}

		for (uint32_t i = 0; i < queueFamilyProperties.size(); i++) {
			if ((queueFlags & queueFamilyProperties[i].queueFlags) == queueFlags) {
				return i;
			}
		}
		throw std::runtime_error("Could not find a matching queue family index");
	}
	/*
	创建queueCI: graphics-compute-transfer
	创建command-pool-graphics
	假设enabledExtensions带有swapchain
	*/
	VkResult VulkanDevice::createLogicalDevice(
		VkPhysicalDeviceFeatures enabledFeatures, std::vector<const char*> enabledExtensions,
		void* nextChain, VkQueueFlags requestedQueueFlags) {

		this->enabledFeatures = enabledFeatures;

		//最优 queue
		std::vector<VkDeviceQueueCreateInfo> queueCIs{};
		const float defaultQueuePriority(0.0f);

		if (VK_QUEUE_GRAPHICS_BIT & requestedQueueFlags) {
			queueFamilyIndices.graphics = getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
			VkDeviceQueueCreateInfo queueInfo{};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = queueFamilyIndices.graphics;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			queueInfo.queueCount = 1;
			queueCIs.push_back(queueInfo);
		}
		else
		{
			queueFamilyIndices.graphics = 0;
		}
		if (VK_QUEUE_COMPUTE_BIT & requestedQueueFlags) {
			queueFamilyIndices.compute = getQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
			// If compute family index differs, we need an additional queue create info for the compute queue
			if (queueFamilyIndices.compute != queueFamilyIndices.graphics) {
				VkDeviceQueueCreateInfo queueInfo{};
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueFamilyIndex = queueFamilyIndices.compute;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueInfo.queueCount = 1;
				queueCIs.push_back(queueInfo);
			}
		}
		else
		{
			queueFamilyIndices.compute = queueFamilyIndices.graphics;
		}

		if (VK_QUEUE_TRANSFER_BIT & requestedQueueFlags) {
			queueFamilyIndices.transfer = getQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
			if (queueFamilyIndices.transfer != queueFamilyIndices.graphics &&
				queueFamilyIndices.transfer != queueFamilyIndices.compute)
			{
				VkDeviceQueueCreateInfo queueInfo{};
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueFamilyIndex = queueFamilyIndices.transfer;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueInfo.queueCount = 1;
				queueCIs.push_back(queueInfo);
			}
		}
		else
		{
			queueFamilyIndices.transfer = queueFamilyIndices.graphics;
		}
		//vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
		//present 使用 graphics
		queueFamilyIndices.present = queueFamilyIndices.graphics;

		std::vector<const char*> deviceExtensions(enabledExtensions);
		if (extensionSupported(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
			deviceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			enableDebugMarkers = true;
		}
		if (deviceExtensions.size() > 0) {
			for (const char* extension : deviceExtensions) {
				if (!extensionSupported(extension)) {
					std::cerr << "Enabled device extension \"" << extension << "\" is not present at device level\n";
				}
			}
		}

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = queueCIs.size();
		createInfo.pQueueCreateInfos = queueCIs.data();
		createInfo.pEnabledFeatures = &enabledFeatures;
		createInfo.enabledExtensionCount = deviceExtensions.size();
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();//应该包含 swapchain

		if (nextChain) {
			VkPhysicalDeviceFeatures2 features2{};
			features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			features2.features = enabledFeatures;
			features2.pNext = nextChain;

			createInfo.pEnabledFeatures = nullptr;
			createInfo.pNext = &features2;
		}
		VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice);

		// pool-graphics command buffers
		commandPool = createCommandPool(queueFamilyIndices.graphics);
		vkGetDeviceQueue(logicalDevice, queueFamilyIndices.graphics, 0, &graphicsQueue);
		vkGetDeviceQueue(logicalDevice, queueFamilyIndices.present, 0, &presentQueue);
		return result;
	}

	VkResult VulkanDevice::createBuffer(
		VkBufferUsageFlags usageFlags,
		VkMemoryPropertyFlags memoryProperties,
		VkDeviceSize size,
		VkBuffer* buffer,
		VkDeviceMemory* deviceMemory,
		void* data)
	{
		VkBufferCreateInfo bufferCI{};
		bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCI.usage = usageFlags;
		bufferCI.size = size;
		bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		MG_CHECK_RESULT(vkCreateBuffer(logicalDevice, &bufferCI, nullptr, buffer));

		VkMemoryRequirements memoryRequire{};
		vkGetBufferMemoryRequirements(logicalDevice, *buffer, &memoryRequire);

		VkMemoryAllocateInfo memoryAI{};
		memoryAI.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAI.allocationSize = memoryRequire.size;
		memoryAI.memoryTypeIndex = getMemoryType(memoryRequire.memoryTypeBits, memoryProperties);
		// If the buffer has VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT set we also need to enable the appropriate flag during allocation
		if (VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT & usageFlags) {
			VkMemoryAllocateFlagsInfo memFlagsInfo{};
			memFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
			memFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
			memoryAI.pNext = &memFlagsInfo;
		}


		MG_CHECK_RESULT(vkAllocateMemory(logicalDevice, &memoryAI, nullptr, deviceMemory));

		MG_CHECK_RESULT(vkBindBufferMemory(logicalDevice, *buffer, *deviceMemory, 0));

		if (data) {
			void* mapped;
			MG_CHECK_RESULT(vkMapMemory(logicalDevice, *deviceMemory, 0, size, 0, &mapped));
			memcpy(mapped, data, size);
			if ((VK_MEMORY_PROPERTY_HOST_COHERENT_BIT & memoryProperties) == 0) {
				VkMappedMemoryRange mappedRange{};
				mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
				mappedRange.memory = *deviceMemory;
				mappedRange.offset = 0;
				mappedRange.size = size;
				MG_CHECK_RESULT(vkFlushMappedMemoryRanges(logicalDevice, 1, &mappedRange));
			}
			vkUnmapMemory(logicalDevice, *deviceMemory);
		}
		return VK_SUCCESS;
	}

	VkResult VulkanDevice::createBuffer(
		VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryProperties, VkDeviceSize size,
		Buffer* buffer, void* data) {

		buffer->device = logicalDevice;

		VkBufferCreateInfo bufferCI{};
		bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCI.usage = usageFlags;
		bufferCI.size = size;
		bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		MG_CHECK_RESULT(vkCreateBuffer(logicalDevice, &bufferCI, nullptr, &buffer->buffer));

		VkMemoryRequirements memReqs{};
		vkGetBufferMemoryRequirements(logicalDevice, buffer->buffer, &memReqs);

		VkMemoryAllocateInfo memoryAI{};
		memoryAI.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAI.allocationSize = memReqs.size;
		memoryAI.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, memoryProperties);
		VkMemoryAllocateFlagsInfo memFlagsInfo{};
		// If the buffer has VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT set we also need to enable the appropriate flag during allocation
		if (VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT & usageFlags) {
			memFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
			memFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
			memoryAI.pNext = &memFlagsInfo;
		}


		MG_CHECK_RESULT(vkAllocateMemory(logicalDevice, &memoryAI, nullptr, &buffer->memory));
		buffer->alignment = memReqs.alignment;
		buffer->size = size;
		buffer->usage = usageFlags;
		buffer->memoryProperties = memoryProperties;

		if (data != nullptr) {
			MG_CHECK_RESULT(buffer->map());
			memcpy(buffer->mapped, data, size);
			if ((VK_MEMORY_PROPERTY_HOST_COHERENT_BIT & memoryProperties) == 0) {
				buffer->flush();
			}
			buffer->unmap();
		}

		// Initialize a default descriptor that covers the whole buffer size
		buffer->setupDescriptor();
		return buffer->bind();
	}

	void VulkanDevice::copyBuffer(Buffer* src, Buffer* dst, VkQueue queue, VkBufferCopy* copyRange)
	{
		assert(dst->size <= src->size);
		assert(src->buffer);
		VkCommandBuffer copyCmd = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
		VkBufferCopy bufferCopy{};
		if (nullptr == copyRange) {
			bufferCopy.size = src->size;
		}
		else
		{
			bufferCopy = *copyRange;
		}
		vkCmdCopyBuffer(copyCmd, src->buffer, dst->buffer, 1, &bufferCopy);

		flushCommandBuffer(copyCmd, queue);
	}
	VkCommandPool VulkanDevice::createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags) {

		VkCommandPoolCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.flags = createFlags;
		createInfo.queueFamilyIndex = queueFamilyIndex;

		VkCommandPool commandPool;
		MG_CHECK_RESULT(vkCreateCommandPool(logicalDevice, &createInfo, nullptr, &commandPool));
		return commandPool;
	}
	void VulkanDevice::createCommandBuffers(VkCommandBufferLevel level, VkCommandBuffer* cmdBuffers, uint32_t count)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = count;
		allocInfo.level = level;

		MG_CHECK_RESULT(vkAllocateCommandBuffers(logicalDevice, &allocInfo, cmdBuffers));
	}
	VkCommandBuffer VulkanDevice::createCommandBuffer(VkCommandBufferLevel level,bool begin) {

		VkCommandBufferAllocateInfo cmdAI{};
		cmdAI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdAI.commandBufferCount = 1;
		cmdAI.commandPool = commandPool;
		cmdAI.level = level;

		VkCommandBuffer cmd;
		MG_CHECK_RESULT(vkAllocateCommandBuffers(logicalDevice, &cmdAI, &cmd));

		if (begin) 
		{
			VkCommandBufferBeginInfo cmdBI{};
			cmdBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			//cmdBI.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			MG_CHECK_RESULT(vkBeginCommandBuffer(cmd, &cmdBI));
		}
		return cmd;
	}

	void VulkanDevice::flushCommandBuffer(VkCommandBuffer cmd, VkQueue queue, bool free) {
		if (VK_NULL_HANDLE == cmd)
		{
			return;
		}
		MG_CHECK_RESULT(vkEndCommandBuffer(cmd));
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd;

		VkFenceCreateInfo fenceCI{};
		fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCI.flags = VK_FLAGS_NONE;
		VkFence fence;
		MG_CHECK_RESULT(vkCreateFence(logicalDevice, &fenceCI, nullptr, &fence));
		MG_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));
		// Wait for the fence to signal that command buffer has finished executing
		MG_CHECK_RESULT(vkWaitForFences(logicalDevice, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));
		//vkResetFences(logicalDevice, 1, &fence);
		vkDestroyFence(logicalDevice, fence, nullptr);
		if (free) {
			vkFreeCommandBuffers(logicalDevice, commandPool, 1, &cmd);
		}
	}
	bool VulkanDevice::extensionSupported(std::string extension) {
		return std::find(supportedExtensions.begin(), supportedExtensions.end(), extension) != supportedExtensions.end();
	}
	VkFormat VulkanDevice::getSupportedDepthFormat(bool checkSamplingSupport) {

		std::vector<VkFormat> formats = {
			VK_FORMAT_D32_SFLOAT,VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,VK_FORMAT_D16_UNORM
		};

		for (const auto& fmt : formats) {
			VkFormatProperties fmtProps;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, fmt, &fmtProps);
			if (VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT & fmtProps.optimalTilingFeatures) {
				if (checkSamplingSupport)
				{
					if (!(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT & fmtProps.optimalTilingFeatures))
					{
						continue;
					}
				}
				return fmt;
			}
		}
		throw std::runtime_error("Could not find a matching depth format");
	}

}
