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
	������ݲ�һ������
	*/
struct Frame
{
	VkDescriptorSet shadow_ubo;
	VkDescriptorSet geo_ubo;				//���� geo-debug
	VkDescriptorSet geo_ubo_ubo;			//���� geo-debug-instancing

	VkDescriptorSet ui_ubo_tex;				//ui
	VkDescriptorSet ui_ubo_shadow;			//��ӰMap��չʾ
	VkDescriptorSet scene_ubo_shadow;		//��������+ShadowMap
	VkDescriptorSet instance_ubo_mips;		//ʵ����
	VkDescriptorSet instance_ubo;			//ʵ����-��Ӱ

	VkDescriptorSet ground_tex;				//tex	�ذ�
	VkDescriptorSet tex_array;				//tex	ģ��
	VkDescriptorSet cube_map;
	VkDescriptorSet tex_3d;

	mg::Buffer ubo_ui;
	mg::Buffer ubo_scene;				//��� �ƹ�
	mg::Buffer ubo_instance;
	mg::Buffer ubo_shadow;
	mg::Buffer ubo_geo;

	bool mips_subView = false;
	//ֻ�ܲ������ڵ�ǰ Command-Buffer �� DescriptorSet
	int opKey_cached;					
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
		/* �˷�ʽ:����2�� �ڴ���������Set,������2��Set��ϳɵ�һ��Set */
		//VkDescriptorSetLayout set_scr[] = { pipes->setLayout_ubo,pipes->setLayout_tex };
		//descriptors::allocateDescriptorSet(set_scr, 2, descriptorPool, device, &set_ui);
		descriptors::allocateDescriptorSet(&pipes->setLayout_ubo, 1, descriptorPool, device, &shadow_ubo);
		descriptors::allocateDescriptorSet(&pipes->setLayout_ubo, 1, descriptorPool, device, &instance_ubo);
		descriptors::allocateDescriptorSet(&pipes->setLayout_ubo_geo, 1, descriptorPool, device, &geo_ubo);
		descriptors::allocateDescriptorSet(&pipes->setLayout_ubo_ubo_geo, 1, descriptorPool, device, &geo_ubo_ubo);

		descriptors::allocateDescriptorSet(&pipes->setLayout_ubo_tex, 1, descriptorPool, device, &ui_ubo_tex);
		descriptors::allocateDescriptorSet(&pipes->setLayout_ubo_tex, 1, descriptorPool, device, &ui_ubo_shadow);
		descriptors::allocateDescriptorSet(&pipes->setLayout_ubo_tex, 1, descriptorPool, device, &scene_ubo_shadow);
		descriptors::allocateDescriptorSet(&pipes->setLayout_ubo_tex, 1, descriptorPool, device, &instance_ubo_mips);
		descriptors::allocateDescriptorSet(&pipes->setLayout_tex, 1, descriptorPool, device, &ground_tex);
		descriptors::allocateDescriptorSet(&pipes->setLayout_tex, 1, descriptorPool, device, &tex_array);
		descriptors::allocateDescriptorSet(&pipes->setLayout_tex, 1, descriptorPool, device, &cube_map);
		descriptors::allocateDescriptorSet(&pipes->setLayout_tex, 1, descriptorPool, device, &tex_3d);

		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSizes[0], &ubo_scene);
		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSizes[1], &ubo_shadow);
		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSizes[2], &ubo_instance);
		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSizes[3], &ubo_ui);

		ubo_scene.map();//����mapped��ַ
		ubo_shadow.map();
		ubo_instance.map();
		ubo_ui.map();
	}
	/* ��set����Դ(ubo,tex) */
	void updateDescritors(VkDevice device, Resource* res)
	{
		/* һЩ UBO */
		std::vector<VkDescriptorType> types;
		std::vector<uint32_t> counts;
		std::vector<void*> infos;
		/* ��ȾShadowMap�� �ƹ���� */
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };
		counts = { 1 };
		infos = { &ubo_shadow.descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), shadow_ubo, device);
		infos = { &ubo_instance.descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), instance_ubo, device);
		infos = { &ubo_scene.descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), geo_ubo, device);//geo-debug
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };
		counts = { 1,1 };
		infos = { &ubo_instance.descriptor,&ubo_scene.descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), geo_ubo_ubo, device);//geo-debug-instancing

		//����+ShadowMap
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		counts = { 1,1 };
		infos = { &ubo_scene.descriptor,&res->tex_shadow->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), scene_ubo_shadow, device);
		//UI: ����+UIͼƬ
		infos = { &ubo_ui.descriptor,&res->tex_ui->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), ui_ubo_tex, device);
		infos = { &ubo_ui.descriptor,&res->tex_shadow->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), ui_ubo_shadow, device);
		//ʵ����: ʵ��λ��+mips
		infos = { &ubo_instance.descriptor,&res->tex_mips->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), instance_ubo_mips, device);

		/* tex-array,cube-map,3D-tex */
		counts = { 1 };
		types = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		infos = { &res->tex_array->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), tex_array, device);
		infos = { &res->tex_cube->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), cube_map, device);
		infos = { &res->tex_3D->texture->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), tex_3d, device);
		infos = { &res->tex_floor->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), ground_tex, device);

	}
	/* �л���ͼ��ʽ */
	void update(VkDevice device, Resource* res)
	{
		if (opKey_cached <= 0)return;

		std::vector<VkDescriptorType> types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		std::vector<uint32_t> counts = { 1, 1 };
		std::vector<void*> infos{};
		switch (opKey_cached)
		{
		case 3:
			mips_subView = !mips_subView;
			if (mips_subView) 
			{
				infos = { &ubo_instance.descriptor, &res->subView->descriptor };
				mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), instance_ubo_mips, device);
			}
			else
			{
				infos = { &ubo_instance.descriptor, &res->tex_mips->descriptor };
				mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), instance_ubo_mips, device);
			}
			break;
		}
		opKey_cached = -1;
	}
	void cleanup(VkDevice device)
	{

		ubo_ui.destroy();
		ubo_scene.destroy();
		ubo_shadow.destroy();
		ubo_instance.destroy();
	}
};