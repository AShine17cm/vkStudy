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
	相关数据此时 可以不存在
	*/
struct Frame
{
	VkDescriptorSet ui;					//ui  shadowMap 展示
	VkDescriptorSet scene_ubo;			//阴影阶段

	VkDescriptorSet geo_mrt_ground;		//tex	地板
	VkDescriptorSet geo_mrt_mips;		//tex	mips
	VkDescriptorSet geo_mrt_texArray;	//tex	模型
	VkDescriptorSet geo_mrt_cubeMap;	//tex   球体

	VkDescriptorSet deferred_compose;	//超级 set

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
		descriptors::allocateDescriptorSet(&pipes->setLayout_ui, 1, descriptorPool, device, &ui);
		descriptors::allocateDescriptorSet(&pipes->setLayout_ubo, 1, descriptorPool, device, &scene_ubo);					//阴影阶段

		descriptors::allocateDescriptorSet(&pipes->setLayout_solid, 1, descriptorPool, device, &geo_mrt_ground);			//geo_mrt 阶段
		descriptors::allocateDescriptorSet(&pipes->setLayout_solid, 1, descriptorPool, device, &geo_mrt_mips);
		descriptors::allocateDescriptorSet(&pipes->setLayout_solid, 1, descriptorPool, device, &geo_mrt_texArray);
		descriptors::allocateDescriptorSet(&pipes->setLayout_solid, 1, descriptorPool, device, &geo_mrt_cubeMap);

		descriptors::allocateDescriptorSet(&pipes->setLayout_deferred_compose, 1, descriptorPool, device, &deferred_compose);//延迟渲染 着色阶段

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
		//UI: 顶点+UI图片
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		infos = { &ubo_ui.descriptor,&res->tex_ui->descriptor,&res->tex_shadow->descriptor };
		counts = { 1,1,1 };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), ui, device);
		/* 阴影阶段: 渲染ShadowMap的 灯光矩阵 */
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };
		counts = { 1 };
		infos = { &ubo_scene.descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), scene_ubo, device);

		/* geo_mrt 法线+颜色 */
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

		/* Compose 阶段 */
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
	/* 切换贴图格式 */
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
