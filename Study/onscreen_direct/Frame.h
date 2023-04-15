/* 用于组织 Material相关数据 */
#pragma once
#include <vector>
#include "vulkan/vulkan.h"
//#include "VulkanTools.h"
#include "descriptors.h"
#include "glm.hpp"
#include "Buffer.h"

#include "PipelineHub.h"
#include "Resource.h"

using namespace glm;
using namespace mg;

struct  uboData
{
	glm::vec4 data;				
};

struct Frame
{
	VkDescriptorSet setScreen;			//data-tex			floor

	mg::Buffer uboScreen;			
	

	void prepare(VulkanDevice* vulkanDevice, VkDescriptorPool descriptorPool, PipelineHub* pipes)
	{
		VkDevice device = vulkanDevice->logicalDevice;
		descriptors::allocateDescriptorSet(&pipes->setLayout_screen, 1, descriptorPool, device, &setScreen);

		VkDeviceSize bufferSize = sizeof(uboData);
		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSize, &uboScreen);			


		uboScreen.map();
		
		uboData colorFloor = {  { 1.0f,0,0,1 } };
		memcpy(uboScreen.mapped, &colorFloor, sizeof(uboData));			
	}
	void updateDescritors(VkDevice device, Resource* res)
	{
		std::vector<VkDescriptorType> types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		std::vector<uint32_t> counts = { 1,1 };
		std::vector<void*> infos = {&uboScreen.descriptor, &res->tex_floor->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(),counts.size(), setScreen, device);
	}
	void cleanup(VkDevice device)
	{
		uboScreen.destroy();
	}
};
