#pragma once
//#include "vulkan/vulkan.h"
#include "macros.h"
#include "VulkanDevice.h"
#include <map>
//Sascha Willems	:	sa
namespace vks 
{
	/*
	Vulkan buffer object
	*/
	struct saBuffer 
	{
		VkDevice device;
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkDescriptorBufferInfo descriptor;
		int32_t count = 0;
		void* mapped = nullptr;
		void create(mg::VulkanDevice* device,
			VkBufferUsageFlags usageFlags,
			VkMemoryPropertyFlags memoryPropertyFlags,
			VkDeviceSize size);

		void destroy();
		void map();
		void unmap();
		void flush(VkDeviceSize size = VK_WHOLE_SIZE);

	};

	//VkPipelineShaderStageCreateInfo loadShader(VkDevice device, std::string filename, VkShaderStageFlagBits stage);
	//void readDirectory(const std::string& directory, const std::string& pattern, std::map<std::string, std::string>& filelist, bool recursive);
}