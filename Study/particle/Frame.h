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

/*	建立DescriptorSet和 UBO+Texture的内存关联
	相关数据不一定存在
	*/
struct Frame
{

	VkDescriptorSet ui;					//ui
	VkDescriptorSet shadow_ubo;			//阴影渲染

	VkDescriptorSet scene_shadow_h;		//阴影合成  管线上的资源继承

	VkDescriptorSet tex_ground;			//tex	地板
	VkDescriptorSet pbr;				//metallic + tex

	mg::Buffer ubo_ui;
	mg::Buffer ubo_scene;				//相机 灯光
	mg::Buffer ubo_pbr;

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
		descriptors::allocateDescriptorSet(&pipes->setLayout_ui, 1, descriptorPool, device, &ui);			//ui
		descriptors::allocateDescriptorSet(&pipes->setLayout_shadow, 1, descriptorPool, device, &shadow_ubo);	//阴影阶段

		descriptors::allocateDescriptorSet(&pipes->setLayout_shadow_h, 1, descriptorPool, device, &scene_shadow_h);//阴影合成
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
		ubo_scene.map();//给出mapped地址
		ubo_ui.map();
		ubo_pbr.map();
	}
	/* 绑定set和资源(ubo,tex) */
	void updateDescritors(VkDevice device, Resource* res)
	{
		/* 一些 UBO */
		std::vector<VkDescriptorType> types;
		std::vector<uint32_t> counts;
		std::vector<void*> infos;

		//UI: 顶点+UI图片
		types = {
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		counts = { 1,1,1 };
		infos = { &ubo_ui.descriptor,&res->tex_ui->descriptor,&res->tex_shadow->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), ui, device);

		/* 渲染ShadowMap的 灯光矩阵 */
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };
		counts = { 1 };
		infos = { &ubo_scene.descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), shadow_ubo, device);

		//场景+ShadowMap
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
