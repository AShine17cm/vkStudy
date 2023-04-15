#pragma once
#include <vector>
#include "vulkan/vulkan.h"
#include "VulkanTools.h"
#include "descriptors.h"
#include "glm.hpp"
#include "Buffer.h"
#include "PipelineHub.h"
#include "Resource.h"

using namespace glm;
using namespace mg;

struct Frame
{
	VkDescriptorSet set_scene;			//scene
	VkDescriptorSet set_pillar;			//tex	ģ��
	VkDescriptorSet set_ground;			//tex	�ذ�
	VkDescriptorSet set_instance;		//ʵ����

	mg::Buffer sceneUbo;				//��� �ƹ�
	mg::Buffer instanceUbo;

	void prepare(VulkanDevice* vulkanDevice, VkDescriptorPool descriptorPool, PipelineHub* pipes,VkDeviceSize* bufferSizes)
	{
		VkDevice device = vulkanDevice->logicalDevice;
		descriptors::allocateDescriptorSet(&pipes->setLayout_ubo, 1, descriptorPool, device, &set_scene);
		descriptors::allocateDescriptorSet(&pipes->setLayout_ubo, 1, descriptorPool, device, &set_instance);
		descriptors::allocateDescriptorSet(&pipes->setLayout_tex, 1, descriptorPool, device, &set_pillar);
		descriptors::allocateDescriptorSet(&pipes->setLayout_tex, 1, descriptorPool, device, &set_ground);

		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSizes[0], &sceneUbo);	

		sceneUbo.map();//����mapped��ַ

		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSizes[1], &instanceUbo);
		instanceUbo.map();
	}
	void updateDescritors(VkDevice device, Resource* res)
	{
		//������Ϣ
		std::vector<VkDescriptorType> types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };
		std::vector<uint32_t> counts = { 1 };
		std::vector<void*> infos = { &sceneUbo.descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), set_scene, device);
		//ʵ����
		infos = {&instanceUbo.descriptor};
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), set_instance, device);

		//ģ��
		types = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		infos = { &res->tex_figure->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), set_pillar, device);

		//�ذ�
		infos = { &res->tex_floor->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), set_ground, device);
	}
	void cleanup(VkDevice device)
	{
		sceneUbo.destroy();
		instanceUbo.destroy();
	}
};
