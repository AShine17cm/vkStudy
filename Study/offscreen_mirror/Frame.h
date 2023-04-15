#pragma once
#include <vector>
#include "vulkan/vulkan.h"
#include "VulkanTools.h"
#include "descriptors.h"
#include "glm.hpp"
#include "Buffer.h"

#include "PipelineHub.h"
#include "Resource.h"
#include "OffscreenPass.h"

using namespace glm;
using namespace mg;

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};
struct UboArray					//做 descriptor count=4 的实验
{
	alignas(64) glm::vec4 data;//需要满足limits
};
struct  BlockColor
{
	glm::vec4 tint[4]; //结构内数组 不需要设定 descriptor-count=4
};
struct Frame
{
	VkDescriptorSet setScene_0;		//scene,A/B			set=0,binding=0
	VkDescriptorSet setA_1;			//array-tint-tex	cubes
	VkDescriptorSet setB_1;			//tint-tex			floor
	VkDescriptorSet setC_1;			//offscr-tex

	mg::Buffer sceneUbo;			//A/B				set=0,binding=0
	mg::Buffer uboArray;			//setA_1			set=1,binding=0
	mg::Buffer tintA;				//setA_1			set=1,binding=1
	mg::Buffer tintB;				//setB_1			set=1,binding=0

	void prepare(VulkanDevice* vulkanDevice, VkDescriptorPool descriptorPool, PipelineHub* pipes)
	{
		VkDevice device = vulkanDevice->logicalDevice;
		descriptors::allocateDescriptorSet(&pipes->setLayout_scene, 1, descriptorPool, device, &setScene_0);
		descriptors::allocateDescriptorSet(&pipes->setLayout_A1, 1, descriptorPool, device, &setA_1);
		descriptors::allocateDescriptorSet(&pipes->setLayout_B1, 1, descriptorPool, device, &setB_1);
		descriptors::allocateDescriptorSet(&pipes->setLayout_B1, 1, descriptorPool, device, &setC_1);

		VkDeviceSize bufferSize = sizeof(UniformBufferObject);
		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSize, &sceneUbo);			//set=0,binding=0

		bufferSize = sizeof(UboArray)*4;
		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSize, &uboArray);			//set=1,binding=0

		bufferSize = sizeof(BlockColor);
		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSize, &tintA);			//set=1,binding=1

		bufferSize = sizeof(BlockColor);
		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSize, &tintB);			//set=1,binding=0

		sceneUbo.map();//给出mapped地址
		uboArray.map();
		tintA.map();
		tintB.map();
		
		UboArray dataArrray[4];				
		dataArrray[0] = { { 1.0f,1.0f,1.0f,1.0f } };
		dataArrray[1] = { { .5f,.5f,.5f,.5f } };
		dataArrray[2] = { { .2f,.2f,.2f,.2f } };
		dataArrray[3] = { { 1.0f,0,0,0 } };
		memcpy(uboArray.mapped, dataArrray, sizeof(UboArray) * 4);//set=1,binding=0
		
		BlockColor colorA = { { { 1.0f,0,0,1 },{0,1.0f,0,1},{0,0,1.0f,1},{0.5f,1.0f,0.5f,1} } };
		memcpy(tintA.mapped, &colorA, sizeof(BlockColor));				//set=1,binding=1

		BlockColor colorB = { { { 1.0f,0,0,1 },{0,1.0f,0,1},{0,0,1.0f,1},{0.5f,1.0f,0.5f,1} } };
		memcpy(tintB.mapped, &colorB, sizeof(BlockColor));				//set=1,binding=0
	}
	void updateDescritors(VkDevice device, Resource* res,OffscreenPass* offscr)
	{
		/* -----------Material A------------ */
		//Set=0,binding=0		场景矩阵		多个shader公用
		std::vector<VkDescriptorType> types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };
		std::vector<uint32_t> counts = { 1};
		std::vector<void*> infos = { &sceneUbo.descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), setScene_0, device);
		
		VkDescriptorBufferInfo infoArray[4];		//准备写有4个descriptor的binding
		infoArray[0] = uboArray.descriptor; infoArray[0].offset = 0;
		infoArray[1] = uboArray.descriptor; infoArray[1].offset = 64;
		infoArray[2] = uboArray.descriptor; infoArray[2].offset = 64 * 2;
		infoArray[3] = uboArray.descriptor; infoArray[3].offset = 64 * 3;
		//Set=1,binding=0,1,2		材质 data-Array + tint + texture
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		counts = {4, 1,1 };
		infos = {infoArray, &tintA.descriptor, &res->tex_figure->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(),counts.size(), setA_1, device);
		
		/* -----------Material B------------- */
		//Set=0,binding=0		场景矩阵		多个shader公用
		//Set=1,binding=0,1		材质 tint + texture
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		counts = { 1,1 };
		infos = { &tintB.descriptor, &res->tex_floor->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), setB_1, device);

		/* ------------ 镜像角度 ------------- */
		infos = {&tintB.descriptor, &offscr->color->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), setC_1, device);
	}
	void cleanup(VkDevice device)
	{
		sceneUbo.destroy();
		uboArray.destroy();
		tintA.destroy();
		tintB.destroy();
	}
};
