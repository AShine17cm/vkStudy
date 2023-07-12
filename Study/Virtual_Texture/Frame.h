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

	VkDescriptorSet plane_ubo;			
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
		descriptors::allocateDescriptorSet(&pipes->setLayout, 1, descriptorPool, device, &plane_ubo);	//阴影阶段

		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSizes[0], &ubo_scene);

		ubo_scene.map();//给出mapped地址
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
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), plane_ubo, device);

		//场景+ShadowMap
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		counts = { 1,1 };
		infos = { &ubo_scene.descriptor,&res->tex_shadow->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), scene_shadow_h, device);
		//pbr
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };
		counts = { 1 };
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
		ubo_scene.destroy();
		ubo_pbr_bg.destroy();
	}
};
