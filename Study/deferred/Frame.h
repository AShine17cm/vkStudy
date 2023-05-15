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

/*	����DescriptorSet�� UBO+Texture���ڴ����
	������ݴ�ʱ ���Բ�����
	*/
struct Frame
{
	VkDescriptorSet ui;					//ui  shadowMap չʾ
	VkDescriptorSet scene_ubo;			//��Ӱ�׶�

	VkDescriptorSet geo_mrt_ground;		//tex	�ذ�
	VkDescriptorSet geo_mrt_mips;		//tex	mips
	VkDescriptorSet geo_mrt_texArray;	//tex	ģ��
	VkDescriptorSet geo_mrt_cubeMap;	//tex   ����

	VkDescriptorSet deferred_compose;	//���� set

	mg::Buffer ubo_ui;
	mg::Buffer ubo_scene;				//��� �ƹ�

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
		descriptors::allocateDescriptorSet(&pipes->setLayout_ui, 1, descriptorPool, device, &ui);
		descriptors::allocateDescriptorSet(&pipes->setLayout_ubo, 1, descriptorPool, device, &scene_ubo);					//��Ӱ�׶�

		descriptors::allocateDescriptorSet(&pipes->setLayout_solid, 1, descriptorPool, device, &geo_mrt_ground);			//geo_mrt �׶�
		descriptors::allocateDescriptorSet(&pipes->setLayout_solid, 1, descriptorPool, device, &geo_mrt_mips);
		descriptors::allocateDescriptorSet(&pipes->setLayout_solid, 1, descriptorPool, device, &geo_mrt_texArray);
		descriptors::allocateDescriptorSet(&pipes->setLayout_solid, 1, descriptorPool, device, &geo_mrt_cubeMap);

		descriptors::allocateDescriptorSet(&pipes->setLayout_deferred_compose, 1, descriptorPool, device, &deferred_compose);//�ӳ���Ⱦ ��ɫ�׶�

		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSizes[0], &ubo_scene);

		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSizes[1], &ubo_ui);

		ubo_scene.map();//����mapped��ַ
		ubo_ui.map();
	}
	/* ��set����Դ(ubo,tex) */
	void updateDescritors(VkDevice device, Resource* res)
	{
		/* һЩ UBO */
		std::vector<VkDescriptorType> types;
		std::vector<uint32_t> counts;
		std::vector<void*> infos;
		//UI: ����+UIͼƬ
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		infos = { &ubo_ui.descriptor,&res->tex_ui->descriptor,&res->tex_shadow->descriptor };
		counts = { 1,1,1 };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), ui, device);
		/* ��Ӱ�׶�: ��ȾShadowMap�� �ƹ���� */
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };
		counts = { 1 };
		infos = { &ubo_scene.descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), scene_ubo, device);

		/* geo_mrt ����+��ɫ */
		counts = { 1,1 };
		types = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		infos = {&res->norm_02->descriptor, &res->tex_mips->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), geo_mrt_mips, device);
		infos = {&res->norm_02->descriptor, &res->tex_array->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), geo_mrt_texArray, device);
		infos = {&res->norm_02->descriptor, &res->tex_cube->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), geo_mrt_cubeMap, device);
		infos = {&res->norm_02->descriptor, &res->tex_floor->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), geo_mrt_ground, device);

		/* Compose �׶� */
		types = {
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ,	//shadow
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ,	//pos
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ,	//normal
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ,	//albedo
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };		//lights,camera
		counts = { 1,1,1,1,1 };
		infos = { &res->tex_shadow->descriptor,
					&res->geo_pos->descriptor,
					&res->geo_normal->descriptor,
					&res->geo_albedo->descriptor,
					&ubo_scene.descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), deferred_compose, device);
	}
	/* �л���ͼ��ʽ */
	void update(VkDevice device, Resource* res)
	{
		if (opKey_cached <= 0)return;

		std::vector<VkDescriptorType> types = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		std::vector<uint32_t> counts = { 1,1 };
		std::vector<void*> infos{};
		switch (opKey_cached)
		{
		case 4:
			mips_subView = !mips_subView;
			if (mips_subView) 
			{
				infos = {&res->norm_01->descriptor, &res->subView->descriptor };
				mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), geo_mrt_mips, device);
			}
			else
			{
				infos = {&res->norm_01->descriptor, &res->tex_mips->descriptor };
				mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), geo_mrt_mips, device);
			}
			break;
		}
		opKey_cached = -1;
	}
	void cleanup(VkDevice device)
	{
		ubo_ui.destroy();
		ubo_scene.destroy();
	}
};
