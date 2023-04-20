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

/* Set �� UBO ��Դ */
struct Frame
{
	VkDescriptorSet set_ui;				//ui
	VkDescriptorSet set_view;			//view-proj-light-camera
	VkDescriptorSet set_instance;		//ʵ����
	VkDescriptorSet set_pillar;			//tex	ģ��
	VkDescriptorSet set_ground;			//tex	�ذ�

	mg::Buffer ui_ubo;
	mg::Buffer view_ubo;				//��� �ƹ�
	mg::Buffer instance_ubo;

	void prepare(VulkanDevice* vulkanDevice, VkDescriptorPool descriptorPool, PipelineHub* pipes,VkDeviceSize* bufferSizes)
	{
		VkDevice device = vulkanDevice->logicalDevice;
		/* �˷�ʽ:����2�� �ڴ���������Set,������2��Set��ϳɵ�һ��Set */
		//VkDescriptorSetLayout set_scr[] = { pipes->setLayout_ubo,pipes->setLayout_tex };
		//descriptors::allocateDescriptorSet(set_scr, 2, descriptorPool, device, &set_ui);

		descriptors::allocateDescriptorSet(&pipes->setLayout_ui, 1, descriptorPool, device, &set_ui);
		descriptors::allocateDescriptorSet(&pipes->setLayout_ubo, 1, descriptorPool, device, &set_view);
		descriptors::allocateDescriptorSet(&pipes->setLayout_ubo, 1, descriptorPool, device, &set_instance);
		descriptors::allocateDescriptorSet(&pipes->setLayout_tex, 1, descriptorPool, device, &set_pillar);
		descriptors::allocateDescriptorSet(&pipes->setLayout_tex, 1, descriptorPool, device, &set_ground);


		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSizes[0], &view_ubo);	
		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSizes[1], &instance_ubo);
		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSizes[2], &ui_ubo);
		view_ubo.map();//����mapped��ַ
		instance_ubo.map();
		ui_ubo.map();
	}
	/* ��set����Դ(ubo,tex) */
	void updateDescritors(VkDevice device, Resource* res)
	{
		//������Ϣ
		std::vector<VkDescriptorType> types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };
		std::vector<uint32_t> counts = { 1 };
		std::vector<void*> infos = { &view_ubo.descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), set_view, device);
		//ʵ����
		infos = {&instance_ubo.descriptor};
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), set_instance, device);

		//ģ��
		types = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		infos = { &res->tex_figure->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), set_pillar, device);

		//�ذ�
		infos = { &res->tex_floor->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), set_ground, device);
		//UI
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		infos = { &ui_ubo.descriptor,&res->tex_ui->descriptor };
		counts = { 1,1 };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), set_ui, device);
	}
	void cleanup(VkDevice device)
	{
		ui_ubo.destroy();
		view_ubo.destroy();
		instance_ubo.destroy();
	}
};
