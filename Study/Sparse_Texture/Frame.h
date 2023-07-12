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
	VkDescriptorSet set_ground;			//tex	�ذ�
	VkDescriptorSet set_virtualTex;		//��������

	mg::Buffer ui_ubo;
	mg::Buffer view_ubo;				//��� �ƹ�

	void prepare(VulkanDevice* vulkanDevice, VkDescriptorPool descriptorPool, PipelineHub* pipes,VkDeviceSize* bufferSizes)
	{
		VkDevice device = vulkanDevice->logicalDevice;

		descriptors::allocateDescriptorSet(&pipes->setLayout_ui, 1, descriptorPool, device, &set_ui);
		descriptors::allocateDescriptorSet(&pipes->setLayout_ubo, 1, descriptorPool, device, &set_view);
		descriptors::allocateDescriptorSet(&pipes->setLayout_tex, 1, descriptorPool, device, &set_ground);
		descriptors::allocateDescriptorSet(&pipes->setLayout_tex, 1, descriptorPool, device, &set_virtualTex);	//��������

		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSizes[0], &view_ubo);	
		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSizes[1], &ui_ubo);
		view_ubo.map();//����mapped��ַ
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

		//�ذ�
		types = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		infos = { &res->tex_floor->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), set_ground, device);
		//��������
		infos = { &res->vt_TextureHub->texture.descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), set_virtualTex, device);
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
	}
};
