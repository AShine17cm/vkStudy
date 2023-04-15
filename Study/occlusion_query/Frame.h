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

struct UniformBufferObject {
	glm::mat4 view;
	glm::mat4 proj;
	glm::vec4 lightPos;
};
struct  BlockColor
{
	glm::vec4 tint;				//结构内数组 不需要设定 descriptor-count=4
};

struct Frame
{
	VkDescriptorSet setScene_0;			//scene,A/B			set=0,binding=0
	VkDescriptorSet setCubes_1;			//tint-tex			cubes
	VkDescriptorSet setFloor_1;			//tint-tex			floor
	VkDescriptorSet setOccluder_1;		//tint				occluder

	mg::Buffer sceneUbo;				//A/B				set=0,binding=0
	mg::Buffer tintCubes;				//setA_1			set=1,binding=1
	mg::Buffer tintFloor;				//setB_1			set=1,binding=0
	mg::Buffer tintOccluder;			

	void prepare(VulkanDevice* vulkanDevice, VkDescriptorPool descriptorPool, PipelineHub* pipes)
	{
		VkDevice device = vulkanDevice->logicalDevice;
		descriptors::allocateDescriptorSet(&pipes->setLayout_Scene, 1, descriptorPool, device, &setScene_0);
		descriptors::allocateDescriptorSet(&pipes->setLayout_Solid, 1, descriptorPool, device, &setCubes_1);
		descriptors::allocateDescriptorSet(&pipes->setLayout_Solid, 1, descriptorPool, device, &setFloor_1);
		descriptors::allocateDescriptorSet(&pipes->setLayout_Occluder, 1, descriptorPool, device, &setOccluder_1);

		VkDeviceSize bufferSize = sizeof(UniformBufferObject);
		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSize, &sceneUbo);			//set=0,binding=0

		bufferSize = sizeof(BlockColor);
		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSize, &tintCubes);			//set=1,binding=1

		bufferSize = sizeof(BlockColor);
		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSize, &tintFloor);			//set=1,binding=0

		bufferSize = sizeof(BlockColor);
		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSize, &tintOccluder);

		sceneUbo.map();		//给出mapped地址
		tintCubes.map();
		tintFloor.map();
		tintOccluder.map();
		
		BlockColor colorCubes = {  { 1.0f,0,0,1 }  };
		memcpy(tintCubes.mapped, &colorCubes, sizeof(BlockColor));				//set=1,binding=1

		BlockColor colorFloor = {  { 1.0f,0,0,1 } };
		memcpy(tintFloor.mapped, &colorFloor, sizeof(BlockColor));				//set=1,binding=0

		BlockColor colorOccluder = { {0,0,0.7f,1} };
		memcpy(tintOccluder.mapped, &colorOccluder, sizeof(BlockColor));
	}
	void updateDescritors(VkDevice device, Resource* res)
	{
		/* -----------Material A------------ */
		//Set=0,binding=0		场景矩阵		多个shader公用
		std::vector<VkDescriptorType> types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };
		std::vector<uint32_t> counts = { 1};
		std::vector<void*> infos = { &sceneUbo.descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), setScene_0, device);
		
		/* ------------- Cubes 的材质 -------------*/
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		counts = { 1,1 };
		infos = {&tintCubes.descriptor, &res->tex_figure->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(),counts.size(), setCubes_1, device);
		
		/* ----------- Floor 的材质 ------------- */
		infos = { &tintFloor.descriptor, &res->tex_floor->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), setFloor_1, device);

		/* ------------ Occluder 的材质 ------------- */
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };
		counts = { 1};
		infos = {&tintOccluder.descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), setOccluder_1, device);
	}
	void cleanup(VkDevice device)
	{
		sceneUbo.destroy();
		tintCubes.destroy();
		tintFloor.destroy();
		tintOccluder.destroy();
	}
};
