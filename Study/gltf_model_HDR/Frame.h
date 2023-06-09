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

/*	����DescriptorSet�� UBO+Texture���ڴ����
	������ݲ�һ������
	*/
struct Frame
{

	VkDescriptorSet shadow_ubo;			//��Ӱ��Ⱦ

	VkDescriptorSet scene_shadow_h;		//��Ӱ�ϳ�  �����ϵ���Դ�̳�

	VkDescriptorSet pbrBasic_bg;
	VkDescriptorSet pbr_Env;
	VkDescriptorSet pbr_IBL_helmet;
	VkDescriptorSet pbr_IBL_dino;
	VkDescriptorSet pbr_IBL_ship1;
	VkDescriptorSet pbr_IBL_ship2;

	VkDescriptorSet hdr_offscreen;
	VkDescriptorSet hdr_bloom;
	VkDescriptorSet ldr;
	VkDescriptorSet blend;

	mg::Buffer ubo_scene;				//��� �ƹ�
	mg::Buffer ubo_pbr_bg;

	mg::Buffer ubo_pbr_helmet;
	mg::Buffer ubo_pbr_dino;
	mg::Buffer ubo_pbr_ship1;
	mg::Buffer ubo_pbr_ship2;

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
		descriptors::allocateDescriptorSet(&pipes->setLayout_shadow, 1, descriptorPool, device, &shadow_ubo);	//��Ӱ�׶�

		descriptors::allocateDescriptorSet(&pipes->setLayout_shadow_h, 1, descriptorPool, device, &scene_shadow_h);//��Ӱ�ϳ�
		descriptors::allocateDescriptorSet(&pipes->setLayout_pbrBasic, 1, descriptorPool, device, &pbrBasic_bg);
		descriptors::allocateDescriptorSet(&pipes->setLayout_pbrEnv, 1, descriptorPool, device, &pbr_Env);

		descriptors::allocateDescriptorSet(&pipes->setLayout_pbrTexs, 1, descriptorPool, device, &pbr_IBL_helmet);
		descriptors::allocateDescriptorSet(&pipes->setLayout_pbrTexs, 1, descriptorPool, device, &pbr_IBL_dino);
		descriptors::allocateDescriptorSet(&pipes->setLayout_pbrTexs, 1, descriptorPool, device, &pbr_IBL_ship1);
		descriptors::allocateDescriptorSet(&pipes->setLayout_pbrTexs, 1, descriptorPool, device, &pbr_IBL_ship2);

		descriptors::allocateDescriptorSet(&pipes->setLayout_hdr_offscreen, 1, descriptorPool, device, &hdr_offscreen);
		descriptors::allocateDescriptorSet(&pipes->setLayout_hdr_bloom, 1, descriptorPool, device, &hdr_bloom);
		descriptors::allocateDescriptorSet(&pipes->setLayout_ldr, 1, descriptorPool, device, &ldr);
		descriptors::allocateDescriptorSet(&pipes->setLayout_blend, 1, descriptorPool, device, &blend);

		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSizes[0], &ubo_scene);

		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			sizeof(geos::PbrBasic), &ubo_pbr_bg);
		//������Ⱦ
		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			sizeof(geos::PbrMaterial), &ubo_pbr_helmet);
		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			sizeof(geos::PbrMaterial), &ubo_pbr_dino);
		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			sizeof(geos::PbrMaterial), &ubo_pbr_ship1);
		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			sizeof(geos::PbrMaterial), &ubo_pbr_ship2);

		ubo_scene.map();//����mapped��ַ
		ubo_pbr_bg.map();

		ubo_pbr_helmet.map();
		ubo_pbr_dino.map();
		ubo_pbr_ship1.map();
		ubo_pbr_ship2.map();
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
		infos = { &ubo_scene.descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), shadow_ubo, device);

		//����+ShadowMap
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
	//����ɫ ʹ����ͼ
	void add_pbrRender(VkDevice device,geos::gltfPbrRender_spec* render,int modelIdx)
	{
		std::vector<VkDescriptorType> types;
		std::vector<uint32_t> counts;
		std::vector<void*> infos;

		//����+������/�߹�+����+OC+�Է��� 
		types = {
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
		};
		counts = { 1,1,1,1,1,1};

		//��Ӧ��ģ�͵���Դ
		mg::Buffer* ubo = nullptr;
		VkDescriptorSet desc_set = nullptr;
		switch (modelIdx)
		{
		case 0:
			ubo = &ubo_pbr_helmet;
			desc_set = pbr_IBL_helmet;
			break;
		case 1:
			ubo = &ubo_pbr_ship1;
			desc_set = pbr_IBL_ship1;
			break;
		case 11:
			ubo = &ubo_pbr_ship2;
			desc_set = pbr_IBL_ship2;
			break;
		case 2:
			ubo = &ubo_pbr_dino;
			desc_set = pbr_IBL_dino;
			break;
		}

		infos = {
			&ubo->descriptor,
			render->colorImg,
			render->isMetallic?render->metalRough: render->specImg,//������/�߹���
			render->normalImg,
			render->ocImg,
			render->mat.emissiveTextureSet==-1? render->emptyImg:render->emissiveImg
		};
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), desc_set, device);

		memcpy(ubo->mapped, &render->mat, sizeof(geos::PbrMaterial));
	}

	void add_hdr(VkDevice device,RenderPassHub* hub)
	{
		std::vector<VkDescriptorType> types;
		std::vector<uint32_t> counts;
		std::vector<void*> infos;

		types = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		counts = { 1 };
		//�߶�̬ ӳ�� �Ͷ�̬����������
		infos = { &hub->msaaTarget.color_resolved->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), hdr_offscreen, device);
		//Bloom �Ľ��
		infos = { &hub->bloomPass.color->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), blend, device);

		types = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		counts = { 1,1 };
		//Bloom ������  ��������
		infos = { &hub->offscreen.color0->descriptor,&hub->offscreen.color1->descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), hdr_bloom, device);
		//ldr ������  �Ͷ�̬ ����
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), ldr, device);
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
		ubo_scene.destroy();
		ubo_pbr_bg.destroy();

		ubo_pbr_helmet.destroy();
		ubo_pbr_dino.destroy();
		ubo_pbr_ship1.destroy();
		ubo_pbr_ship2.destroy();
	}
};
