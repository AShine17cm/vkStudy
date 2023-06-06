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
#include "PbrEnv.h"

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

	VkDescriptorSet pbrBasic_bg;
	VkDescriptorSet pbrBasic;				//metallic + tex
	VkDescriptorSet pbr_Env;
	VkDescriptorSet pbr_IBL_dino;

	mg::Buffer ubo_ui;
	mg::Buffer ubo_scene;				//相机 灯光
	mg::Buffer ubo_pbr;
	mg::Buffer ubo_pbr_bg;
	mg::Buffer ubo_pbr_albedo;

	mg::Buffer ubo_pbr_dino;

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
		descriptors::allocateDescriptorSet(&pipes->setLayout_pbrBasic, 1, descriptorPool, device, &pbrBasic);
		descriptors::allocateDescriptorSet(&pipes->setLayout_pbrBasic, 1, descriptorPool, device, &pbrBasic_bg);
		descriptors::allocateDescriptorSet(&pipes->setLayout_pbrEnv, 1, descriptorPool, device, &pbr_Env);
		descriptors::allocateDescriptorSet(&pipes->setLayout_pbrTexs, 1, descriptorPool, device, &pbr_IBL_dino);

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
		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			sizeof(geos::PbrBasic), &ubo_pbr_bg);
		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			sizeof(geos::PbrBasic), &ubo_pbr_albedo);
		//单次渲染
		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			sizeof(geos::PbrMaterial), &ubo_pbr_dino);

		ubo_scene.map();//给出mapped地址
		ubo_ui.map();
		ubo_pbr.map();
		ubo_pbr_bg.map();
		ubo_pbr_albedo.map();
		ubo_pbr_dino.map();
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
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };
		counts = { 1 };
		infos = { &ubo_pbr.descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), pbrBasic, device);
		infos = { &ubo_pbr_bg.descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), pbrBasic_bg, device);



	}
	void add_pbrEnv(VkDevice device, PbrEnv* env)
	{
		std::vector<VkDescriptorType> types;
		std::vector<uint32_t> counts = { 1,1,1 };
		std::vector<void*> infos = 
		{
			&env->irradianceCube.descriptor,
			&env->lutBrdf.descriptor,
			&env->prefilteredCube.descriptor 
		};
		types = 
		{
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
		};
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), pbr_Env, device);
	}
	//基础色 使用贴图
	void add_pbrRender(VkDevice device,geos::gltfPbrRender_spec* render,int modelIdx)
	{
		std::vector<VkDescriptorType> types;
		std::vector<uint32_t> counts;
		std::vector<void*> infos;

		//纹理+金属度/高光+法线+OC+自发光 
		types = {
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
		};
		counts = { 1,1,1,1,1,1};
		infos = {
			&ubo_pbr_dino.descriptor,
			render->colorImg,
			render->isMetallic?render->metalRough: render->specImg,
			render->normalImg,
			render->ocImg,
			render->mat.emissiveTextureSet==-1? render->emptyImg:render->emissiveImg
		};
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), pbr_IBL_dino, device);

		memcpy(ubo_pbr_dino.mapped, &render->mat, sizeof(geos::PbrMaterial));
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
		ubo_pbr_bg.destroy();
		ubo_pbr_albedo.destroy();
		ubo_pbr_dino.destroy();
	}
};
