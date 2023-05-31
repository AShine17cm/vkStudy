#pragma once
#include <vector>
#include "vulkan/vulkan.h"
#include "VulkanTools.h"
#include "descriptors.h"
#include "glm.hpp"
#include "Buffer.h"
#include "PipelineHub.h"
#include "Resource.h"
#include "PerObjectData.h"

using namespace glm;
using namespace mg;

/*	����DescriptorSet�� UBO+Texture���ڴ����
	������ݲ�һ������
	*/
struct Frame
{

	VkDescriptorSet ui;					//ui
	VkDescriptorSet shadow_ubo;			//��Ӱ��Ⱦ

	VkDescriptorSet scene_shadow_h;		//��Ӱ�ϳ�  �����ϵ���Դ�̳�

	VkDescriptorSet tex_ground;			//tex	�ذ�
	VkDescriptorSet pbr;				//metallic + tex

	mg::Buffer ubo_ui;
	mg::Buffer ubo_scene;				//��� �ƹ�
	mg::Buffer ubo_pbr;

	bool mips_subView = false;
	int opKey_cached;					//ֻ�ܲ������ڵ�ǰ Command-Buffer �� DescriptorSet
	void CacheKey(int opKey) 
	{
		if (opKey > 0 ) 
		{
			opKey_cached = opKey;
		}
	}
	void prepare(VulkanDevice* vulkanDevice, VkDescriptorPool descriptorPool, PipelineHub* pipes, VkDeviceSize* bufferSizes)
	{
		VkDevice device = vulkanDevice->logicalDevice;
		descriptors::allocateDescriptorSet(&pipes->setLayout_ui, 1, descriptorPool, device, &ui);			//ui
		descriptors::allocateDescriptorSet(&pipes->setLayout_shadow, 1, descriptorPool, device, &shadow_ubo);	//��Ӱ�׶�

		descriptors::allocateDescriptorSet(&pipes->setLayout_shadow_h, 1, descriptorPool, device, &scene_shadow_h);//��Ӱ�ϳ�
		descriptors::allocateDescriptorSet(&pipes->setLayout_solidTex, 1, descriptorPool, device, &tex_ground);
		descriptors::allocateDescriptorSet(&pipes->setLayout_pbrBasic, 1, descriptorPool, device, &pbr);

		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSizes[0], &ubo_scene);

		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSizes[1], &ubo_ui);

		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			sizeof(geos::PbrBasic), &ubo_pbr);
		ubo_scene.map();//����mapped��ַ
		ubo_ui.map();
		ubo_pbr.map();
	}
	/* ��set����Դ(ubo,tex) */
	void updateDescritors(VkDevice device, Resource* res)
	{
		/* һЩ UBO */
		std::vector<VkDescriptorType> types;
		std::vector<uint32_t> counts;
		std::vector<void*> infos;

		//UI: ����+UIͼƬ
		types = {
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		counts = { 1,1,1 };
		infos = { &ubo_ui.descriptor,&res->tex_ui->descriptor,&res->tex_shadow->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), ui, device);

		/* ��ȾShadowMap�� �ƹ���� */
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };
		counts = { 1 };
		infos = { &ubo_scene.descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), shadow_ubo, device);

		//����+ShadowMap
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		counts = { 1,1 };
		infos = { &ubo_scene.descriptor,&res->tex_shadow->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), scene_shadow_h, device);
		//pbr
		infos = { &ubo_pbr.descriptor,&res->tex_floor->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), pbr, device);

		/* shadow-array, tex-mips,tex-array,cube-map,3D-tex */
		counts = { 1 };
		types = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		infos = { &res->tex_floor->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), tex_ground, device);
	}
	/* �л���ͼ��ʽ */
	void update(VkDevice device, Resource* res)
	{
		if (opKey_cached <= 0)return;

		std::vector<VkDescriptorType> types = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		std::vector<uint32_t> counts = { 1 };
		std::vector<void*> infos{};
		switch (opKey_cached)
		{
		case 3:
			//mips_subView = !mips_subView;

			break;
		}
		opKey_cached = -1;
	}
	void cleanup(VkDevice device)
	{
		ubo_ui.destroy();
		ubo_scene.destroy();
		ubo_pbr.destroy();
	}
};
