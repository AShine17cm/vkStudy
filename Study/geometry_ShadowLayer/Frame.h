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

/*	建立DescriptorSet和 UBO+Texture的内存关联
	相关数据不一定存在
	*/
struct Frame
{
	VkDescriptorSet shadow_ubo;

	VkDescriptorSet ui_ubo_tex;				//ui
	VkDescriptorSet ui_shadow;				//阴影Map的展示
	VkDescriptorSet scene_ubo_shadow;		//场景数据+ShadowMap

	VkDescriptorSet ground_tex;				//tex	地板
	VkDescriptorSet tex_mips;				//tex	mips
	VkDescriptorSet tex_array;				//tex	模型
	VkDescriptorSet cube_map;				//tex   球体
	VkDescriptorSet tex_3d;					//tex   3D 纹理

	mg::Buffer ubo_ui;
	mg::Buffer ubo_scene;				//相机 灯光

	bool mips_subView = false;
	int opKey_cached;					//只能操作属于当前 Command-Buffer 的 DescriptorSet
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
		/* 此方式:生成2个 内存上连续的Set,而不是2个Set组合成的一个Set */
		//VkDescriptorSetLayout set_scr[] = { pipes->setLayout_ubo,pipes->setLayout_tex };
		//descriptors::allocateDescriptorSet(set_scr, 2, descriptorPool, device, &set_ui);
		descriptors::allocateDescriptorSet(&pipes->setLayout_ubo, 1, descriptorPool, device, &shadow_ubo);

		descriptors::allocateDescriptorSet(&pipes->setLayout_ubo_tex, 1, descriptorPool, device, &ui_ubo_tex);
		descriptors::allocateDescriptorSet(&pipes->setLayout_ubo_tex, 1, descriptorPool, device, &scene_ubo_shadow);
		descriptors::allocateDescriptorSet(&pipes->setLayout_tex, 1, descriptorPool, device, &ui_shadow);//UI 展示 使用单独的一个 set
		descriptors::allocateDescriptorSet(&pipes->setLayout_tex, 1, descriptorPool, device, &ground_tex);
		descriptors::allocateDescriptorSet(&pipes->setLayout_tex, 1, descriptorPool, device, &tex_mips);
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
			bufferSizes[1], &ubo_ui);

		ubo_scene.map();//给出mapped地址
		ubo_ui.map();
	}
	/* 绑定set和资源(ubo,tex) */
	void updateDescritors(VkDevice device, Resource* res)
	{
		/* 一些 UBO */
		std::vector<VkDescriptorType> types;
		std::vector<uint32_t> counts;
		std::vector<void*> infos;
		/* 渲染ShadowMap的 灯光矩阵 */
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };
		counts = { 1 };
		infos = { &ubo_scene.descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), shadow_ubo, device);

		//场景+ShadowMap
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		counts = { 1,1 };
		infos = { &ubo_scene.descriptor,&res->tex_shadow->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), scene_ubo_shadow, device);
		//UI: 顶点+UI图片
		infos = { &ubo_ui.descriptor,&res->tex_ui->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), ui_ubo_tex, device);

		/* shadow-array, tex-mips,tex-array,cube-map,3D-tex */
		counts = { 1 };
		types = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		infos = { &res->tex_shadow->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), ui_shadow, device);

		infos = {&res->tex_mips->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), tex_mips, device);
		infos = { &res->tex_array->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), tex_array, device);
		infos = { &res->tex_cube->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), cube_map, device);
		infos = { &res->tex_3D->texture->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), tex_3d, device);
		infos = { &res->tex_floor->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), ground_tex, device);

	}
	/* 切换贴图格式 */
	void update(VkDevice device, Resource* res)
	{
		if (opKey_cached <= 0)return;

		std::vector<VkDescriptorType> types = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		std::vector<uint32_t> counts = { 1 };
		std::vector<void*> infos{};
		switch (opKey_cached)
		{
		case 3:
			mips_subView = !mips_subView;
			if (mips_subView) 
			{
				infos = { &res->subView->descriptor };
				mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), tex_mips, device);
			}
			else
			{
				infos = { &res->tex_mips->descriptor };
				mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), tex_mips, device);
			}
			break;
		}
		opKey_cached = -1;
	}
	void cleanup(VkDevice device)
	{

		ubo_ui.destroy();
		ubo_scene.destroy();
		//ubo_shadow.destroy();
	}
};
