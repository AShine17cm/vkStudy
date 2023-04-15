//#include "pch.h"
#include <assert.h>
#include "vulkan/vulkan.h"
#include "Buffer.h"
#include "VulkanTools.h"
namespace mg
{
	VkResult Buffer::map(VkDeviceSize size, VkDeviceSize offset) {
		return vkMapMemory(device, memory, offset, size, 0, &mapped);
	}

	void Buffer::unmap() {
		if (mapped) {
			vkUnmapMemory(device, memory);
			mapped = nullptr;
		}
	}
	VkResult Buffer::bind(VkDeviceSize offset) {
		return vkBindBufferMemory(device, buffer, memory, offset);
	}
	void Buffer::setupDescriptor(VkDeviceSize size, VkDeviceSize offset) {

		descriptor.buffer = buffer;
		descriptor.offset = offset;
		descriptor.range = size;
		//如果buffer的DescriptorType为dynamic, range需设置为单个stride,而不是buffer的整体大小
		//在command buffer中绑定时 会根据物体每次偏移stride大小
	}
	void Buffer::copyTo(void* data, VkDeviceSize size) {
		assert(mapped);
		memcpy(mapped, data, size);
	}
	VkResult Buffer::flush(VkDeviceSize size, VkDeviceSize offset) {
		//if ((VK_MEMORY_PROPERTY_HOST_COHERENT_BIT & memoryProperties) == 0) 
		VkMappedMemoryRange range{};
		range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		range.memory = memory;
		range.size = size;
		range.offset = offset;
		return vkFlushMappedMemoryRanges(device, 1, &range);
	}
	VkResult Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset) {
		//if ((VK_MEMORY_PROPERTY_HOST_COHERENT_BIT & memoryProperties) == 0)
		VkMappedMemoryRange range{};
		range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		range.memory = memory;
		range.size = size;
		range.offset = offset;
		return vkInvalidateMappedMemoryRanges(device, 1, &range);
	}
	void Buffer::destroy() {
		if (buffer) {
			vkDestroyBuffer(device, buffer, nullptr);
		}
		if (memory) {
			vkFreeMemory(device, memory, nullptr);
		}
	}
}
