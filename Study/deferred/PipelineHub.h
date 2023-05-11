#pragma once
#include <vector>
#include "vulkan/vulkan.h"
#include "VulkanTools.h"
#include "glm.hpp"
#include "RenderPassHub.h"
using namespace glm;

struct PipelineHub
{
	/* ������ set-layout */
	VkDescriptorSetLayout setLayout_ui;
	VkDescriptorSetLayout setLayout_ubo;				//��Ӱ�׶�
	VkDescriptorSetLayout setLayout_solid;				//geo_mrt�׶�: <normal,tex>
	VkDescriptorSetLayout setLayout_deferred_compose;	//compose�׶� shadow,pos,norm,albedo 4��tex + һ�����Ե�ubo
	/* set-layout ��ϵ� pipe-layout */
	VkPipelineLayout piLayout_ui;
	VkPipelineLayout piLayout_ubo;				//��Ӱ��Ⱦ
	VkPipelineLayout piLayout_geo_mrt_solid;	//geo-mrt
	VkPipelineLayout piLayout_deferred_compose;	//�ӳ���Ⱦ ������ɫ
	/* ʹ��ͬһ�� pipeline-layout */
	VkPipeline pi_ui;						//shader::<ui.shader>
	VkPipeline pi_shadow;					//shader::<shadowCaster.shader>

	VkPipeline pi_geo_mrt_tex;				//shader::<tex.shader>
	VkPipeline pi_geo_mrt_texArray;			//shader::<texArray.shader>
	VkPipeline pi_geo_mrt_cubeMap;			//shader::<texCube.shader>

	VkPipeline pi_deferred_compose;			//�ϳ���ɫ�Ĺ���	

	void prepare(VkDevice device, RenderPassHub* passHub, uint32_t constantSize)
	{
		VkShaderStageFlags stageVGF = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		/* Descriptor-Layout */
		std::vector<VkDescriptorType> types;
		std::vector<VkShaderStageFlags> stages;
		std::vector<uint32_t> desCounts;
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };
		stages = { stageVGF };
		desCounts = { 1 };
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_ubo);
		//normal+tex
		types = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		stages = { VK_SHADER_STAGE_FRAGMENT_BIT ,VK_SHADER_STAGE_FRAGMENT_BIT };
		desCounts = { 1,1 };
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_solid);
		//ui
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		stages = { stageVGF,
			VK_SHADER_STAGE_FRAGMENT_BIT ,
			VK_SHADER_STAGE_FRAGMENT_BIT };
		desCounts = { 1,1,1 };
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_ui);
		//deferred
		types = {
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ,//shadow
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ,//pos
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ,//normal
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ,//albedo
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };
		stages = {
			VK_SHADER_STAGE_FRAGMENT_BIT ,
			VK_SHADER_STAGE_FRAGMENT_BIT ,
			VK_SHADER_STAGE_FRAGMENT_BIT ,
			VK_SHADER_STAGE_FRAGMENT_BIT ,
			VK_SHADER_STAGE_FRAGMENT_BIT };
		desCounts = { 1,1,1,1,1 };
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_deferred_compose);
		/* Pipeline-Layout */
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		VkPushConstantRange pushRange = { stageVGF,0,constantSize };
		//ui����+��ͼ
		VkDescriptorSetLayout setL_uiTex[] = { setLayout_ui };
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = setL_uiTex;
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_ui));
		//��Ӱ��Ⱦ::    ���ƹ�ռ�ľ���
		VkDescriptorSetLayout setL_shadow[] = { setLayout_ubo };
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = setL_shadow;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_ubo));

		//geo_mrt�׶�:: ����ubo  /+/  normal+tex      
		VkDescriptorSetLayout setL_SolidTex[] = { setLayout_ubo,setLayout_solid };
		pipelineLayoutInfo.setLayoutCount = 2;
		pipelineLayoutInfo.pSetLayouts = setL_SolidTex;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_geo_mrt_solid));
		//compose�׶�
		VkDescriptorSetLayout setL_Deferred[] = { setLayout_deferred_compose };
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = setL_Deferred;
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_deferred_compose));

		/* Pipeline-s */
		std::vector<VkShaderStageFlagBits> shaderStages;
		std::vector<const char*> shaderFiles;
		/* ui */
		shaderStages= { VK_SHADER_STAGE_VERTEX_BIT,VK_SHADER_STAGE_FRAGMENT_BIT };
		shaderFiles = { "shaders/ui.vert.spv","shaders/ui.frag.spv" };
		createPipeline(device, passHub->deferredPass, &shaderFiles, shaderStages, &piLayout_ui, &pi_ui, pipelines::MgPipelineEx::UI);
		/* ��Ӱ�׶� geometry-shader ���Դ��Ӱ */
		shaderStages = { VK_SHADER_STAGE_VERTEX_BIT,VK_SHADER_STAGE_GEOMETRY_BIT };
		shaderFiles = { "shaders/shadowCaster.vert.spv","shaders/shadowCaster.geom.spv" };
		createPipeline(device, passHub->shadowPass, &shaderFiles, shaderStages, &piLayout_ubo, &pi_shadow, pipelines::MgPipelineEx::ShadowMap);

		/* geo_mrt �׶� */
		shaderStages= { VK_SHADER_STAGE_VERTEX_BIT,VK_SHADER_STAGE_FRAGMENT_BIT };
		shaderFiles = { "shaders/geo_mrt.vert.spv", "shaders/geo_mrt_tex.frag.spv" };
		createPipeline(device, passHub->geo_mrt_Pass, &shaderFiles, shaderStages, &piLayout_geo_mrt_solid, &pi_geo_mrt_tex, pipelines::MgPipelineEx::Geo_MRT);
		shaderFiles = { "shaders/geo_mrt.vert.spv", "shaders/geo_mrt_texArray.frag.spv" };
		createPipeline(device, passHub->geo_mrt_Pass, &shaderFiles, shaderStages, &piLayout_geo_mrt_solid, &pi_geo_mrt_texArray, pipelines::MgPipelineEx::Geo_MRT);
		shaderFiles = { "shaders/geo_mrt.vert.spv", "shaders/geo_mrt_cubeMap.frag.spv" };
		createPipeline(device, passHub->geo_mrt_Pass, &shaderFiles, shaderStages, &piLayout_geo_mrt_solid, &pi_geo_mrt_cubeMap, pipelines::MgPipelineEx::Geo_MRT);
		/* compose�׶� */
		shaderStages = { VK_SHADER_STAGE_VERTEX_BIT ,VK_SHADER_STAGE_FRAGMENT_BIT };
		shaderFiles = { "shaders/deferred_compose.vert.spv","shaders/deferred_compose.frag.spv" };
		createPipeline(device, passHub->deferredPass, &shaderFiles, shaderStages, &piLayout_deferred_compose, &pi_deferred_compose, pipelines::MgPipelineEx::Deferred_Compose);
	}
	/* ���� shader ���� pipeline */
	void createPipeline(VkDevice device, VkRenderPass renderPass,
		std::vector<const char*>* shaderFiles,
		std::vector<VkShaderStageFlagBits> shaderStageFlags,
		VkPipelineLayout* piLayout, VkPipeline* pi,
		pipelines::MgPipelineEx kind = pipelines::MgPipelineEx::None)
	{
		std::vector<VkShaderModule> shaderModules;
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		//���ļ��� ���� spv��ʽ�� shader
		for (uint32_t i = 0; i < (*shaderFiles).size(); i++)
		{
			VkShaderModule module = mg::shaders::loadShader((*shaderFiles)[i], device);
			VkPipelineShaderStageCreateInfo shaderStage = {};
			shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStage.stage = shaderStageFlags[i];
			shaderStage.module = module;
			shaderStage.pName = "main";
			assert(shaderStage.module != VK_NULL_HANDLE);

			shaderModules.push_back(module); //�洢�� ����
			shaderStages.push_back(shaderStage);
		}
		/* Vertex */
		std::vector<VkVertexInputAttributeDescription> attributes;
		std::vector<VkVertexInputBindingDescription> bindings;
		VkPipelineVertexInputStateCreateInfo vertexInputSCI{};
		vertexInputSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputSCI = geos::Geo::getVertexInput(&attributes, &bindings);

		/* Color Blend*/
		std::vector<VkPipelineColorBlendAttachmentState> blends{};
		VkPipelineColorBlendAttachmentState blend{};
		blend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blend.blendEnable = VK_FALSE;
		blends.push_back(blend);
		if (pipelines::MgPipelineEx::Geo_MRT == kind)
		{
			//pos,normal.tangent
			blends.push_back(blend);
			blends.push_back(blend);
		}

		std::vector<pipelines::MgPipelineEx> extends{};
		/* һЩ���⴦���߼� */
		switch (kind)
		{
			/* UI������ ��Descriptor-Set�У�����vertex */
		case pipelines::MgPipelineEx::UI:
		case pipelines::MgPipelineEx::Deferred_Compose:
			vertexInputSCI = {};
			vertexInputSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			break;
		case pipelines::MgPipelineEx::ShadowMap:
			extends.push_back(kind);
			break;
		}

		mg::pipelines::createPipeline(
			shaderStages.data(), shaderStages.size(),
			blends.data(), blends.size(),
			&vertexInputSCI,
			*piLayout,
			renderPass,
			device,
			nullptr,
			pi,
			nullptr, extends.data(), extends.size());

		//����pipeline��ϣ�����shader-module
		for (uint32_t i = 0; i < shaderModules.size(); i++)
		{
			vkDestroyShaderModule(device, shaderModules[i], nullptr);
		}
		shaderModules.clear();
		shaderStages.clear();
	}
	void cleanup(VkDevice device)
	{
		vkDestroyPipeline(device, pi_ui, nullptr);
		vkDestroyPipeline(device, pi_shadow, nullptr);					//ͼ�ι��� shadow
		vkDestroyPipeline(device, pi_geo_mrt_tex, nullptr);
		vkDestroyPipeline(device, pi_geo_mrt_texArray, nullptr);
		vkDestroyPipeline(device, pi_geo_mrt_cubeMap, nullptr);
		vkDestroyPipeline(device, pi_deferred_compose, nullptr);

		vkDestroyPipelineLayout(device, piLayout_ui, nullptr);
		vkDestroyPipelineLayout(device, piLayout_ubo, nullptr);			//��Ӱ��Ⱦ �ƹ����
		vkDestroyPipelineLayout(device, piLayout_geo_mrt_solid, nullptr);
		vkDestroyPipelineLayout(device, piLayout_deferred_compose, nullptr);

		vkDestroyDescriptorSetLayout(device, setLayout_ui, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_ubo, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_solid, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_deferred_compose, nullptr);
	}
};
