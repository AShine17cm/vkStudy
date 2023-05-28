#pragma once
#include <vector>
#include "vulkan/vulkan.h"
#include "VulkanTools.h"
#include "glm.hpp"
#include "RenderPassHub.h"
#include "VulkanglTFModel.h"

using namespace glm;

struct PipelineHub
{
	/* ������ set-layout */
	VkDescriptorSetLayout setLayout_ui;
	VkDescriptorSetLayout setLayout_shadow;		//��Ӱ��Ⱦ 
	VkDescriptorSetLayout setLayout_shadow_h;	//��Ӱ�ϳ� h:holder  �����ϵ���Դ�̳�
	VkDescriptorSetLayout setLayout_solidTex;	//����� albedo
	/* set-layout ��ϵ� pipe-layout */
	VkPipelineLayout piLayout_ui;
	VkPipelineLayout piLayout_shadow;		//��Ӱ��Ⱦ
	VkPipelineLayout piLayout_shadow_h;		//��Ӱ�ϳ�: h��ʾռλ�����ں�����Ⱦ�ļ̳�
	VkPipelineLayout piLayout_solid;		//��������+ShadowMap + Object��ͼ  /+/  ui����+ui��ͼ+shadow:2d-array

	VkPipeline pi_ui;						//shader::<ui.shader>
	VkPipeline pi_shadow;					//shader::<shadowCaster.shader>
	VkPipeline pi_shadow_gltf;				//gltf �ļ����Լ��Ķ������Բ���
	/* ʹ��ͬһ�� pipeline-layout */
	VkPipeline pi_Tex;						//shader::<tex.shader>
	VkPipeline pi_Pbr;						//shader::<pbr.shader>	��gltf ģ�� ��PBR ����

	void prepare(VkDevice device, RenderPassHub* passHub, uint32_t constantSize)
	{
		VkShaderStageFlags stageVGF = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		/* Descriptor-Layout */
		std::vector<VkDescriptorType> types;
		std::vector<VkShaderStageFlags> stages;
		std::vector<uint32_t> desCounts;
		/* ui  ��Ӱչʾ */
		types = { 
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		stages = { 
			stageVGF, 
			VK_SHADER_STAGE_FRAGMENT_BIT ,
			VK_SHADER_STAGE_FRAGMENT_BIT };
		desCounts = { 1,1,1 };
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_ui);
		/* ��Ӱ��Ⱦ */
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };
		stages = { stageVGF };
		desCounts = { 1 };
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_shadow);
		// <texture>
		types = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		stages = { VK_SHADER_STAGE_FRAGMENT_BIT };
		desCounts = { 1 };
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_solidTex);
		//��Ӱ�ϳ�
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		stages = { stageVGF , VK_SHADER_STAGE_FRAGMENT_BIT };
		desCounts = { 1,1 };
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_shadow_h);

		/* Pipeline-Layout */
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		VkPushConstantRange pushRange = { stageVGF,0,constantSize };
		//ui ��Ӱչʾ
		VkDescriptorSetLayout setL_ui[] = { setLayout_ui };
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = setL_ui;
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_ui));
		//��Ӱ��Ⱦ::    ���ƹ�ռ�ľ���
		VkDescriptorSetLayout setL_shadow[] = { setLayout_shadow };
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = setL_shadow;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_shadow));

		//��Ӱ�ϳ�::  ����pipeline-layout ����Դ�̳�
		VkDescriptorSetLayout setL_shadow_holder[] = { setLayout_shadow_h };
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = setL_shadow_holder;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;		//�����ã��޷��� layout-scene ����
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_shadow_h));

		//��Ⱦ�׶�::  ������Ϣ+Shadow  /+/  Object��ͼ
		VkDescriptorSetLayout setL_solidTex[] = { setLayout_shadow_h,setLayout_solidTex };
		pipelineLayoutInfo.setLayoutCount = 2;
		pipelineLayoutInfo.pSetLayouts = setL_solidTex;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_solid));

		/* Pipeline-s */
		std::vector<VkShaderStageFlagBits> shaderStages; 
		std::vector<const char*> shaderFiles;
		/* ���㲼�� */
		std::vector<VkVertexInputAttributeDescription> attributes;
		std::vector<VkVertexInputBindingDescription> bindings;
		VkPipelineVertexInputStateCreateInfo vertexInputSCI{};
		vertexInputSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputSCI = geos::Geo::getVertexInput(&attributes, &bindings);
		/* ���㲼�� UI������ ��Descriptor-Set�У�����vertex */
		VkPipelineVertexInputStateCreateInfo vertexInputSCI_ui{};
		vertexInputSCI_ui.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		/* ���㲼�� gltf */
		VkVertexInputBindingDescription vertexInputBinding = { 0, sizeof(vkglTF::Model::Vertex), VK_VERTEX_INPUT_RATE_VERTEX };
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
			{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },						//λ��
			{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3 },		//����
			{ 2, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 6 },			//uv 0
			{ 3, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 8 },			//uv 1
			{ 4, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 10 },	//joint 0
			{ 5, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 14 },	//weight 0
			{ 6, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 18 }		//��ɫ
		};
		VkPipelineVertexInputStateCreateInfo vertexInputSCI_gltf{};
		vertexInputSCI_gltf.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputSCI_gltf.vertexBindingDescriptionCount = 1;
		vertexInputSCI_gltf.pVertexBindingDescriptions = &vertexInputBinding;
		vertexInputSCI_gltf.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
		vertexInputSCI_gltf.pVertexAttributeDescriptions = vertexInputAttributes.data();
		vertexInputSCI_gltf.flags = 0;
		//ui
		shaderStages= { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT };
		shaderFiles = { "shaders/ui.vert.spv","shaders/ui.frag.spv" };
		createPipeline(device, passHub->renderPass, &shaderFiles, shaderStages, &piLayout_ui, &pi_ui, &vertexInputSCI_ui);
		/* ��Ӱ�׶� ʹ�ö��Ƶ�RenderPass */
		shaderStages = { VK_SHADER_STAGE_VERTEX_BIT,VK_SHADER_STAGE_GEOMETRY_BIT };
		shaderFiles = { "shaders/shadowCaster.vert.spv","shaders/shadowCaster.geom.spv" };
		createPipeline(device, passHub->shadowPass, &shaderFiles, shaderStages, &piLayout_shadow, &pi_shadow,&vertexInputSCI, pipelines::MgPipelineEx::ShadowMap);
		shaderFiles = { "shaders/shadowCaster_gltf.vert.spv","shaders/shadowCaster.geom.spv" };
		createPipeline(device, passHub->shadowPass, &shaderFiles, shaderStages, &piLayout_shadow, &pi_shadow_gltf,&vertexInputSCI_gltf, pipelines::MgPipelineEx::ShadowMap);
		/* ��Ⱦ�����׶� */
		shaderStages = { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT };
		shaderFiles = { "shaders/tex.vert.spv", "shaders/tex.frag.spv" };
		createPipeline(device, passHub->renderPass, &shaderFiles,shaderStages, &piLayout_solid, &pi_Tex,&vertexInputSCI);
		/* pbr */
		shaderFiles = { "shaders/pbr.vert.spv", "shaders/pbr.frag.spv" };
		createPipeline(device, passHub->renderPass, &shaderFiles, shaderStages, &piLayout_solid, &pi_Pbr,&vertexInputSCI_gltf);
	}
	/* ���� shader ���� pipeline */
	void createPipeline(VkDevice device, VkRenderPass renderPass, 
		std::vector<const char*>* shaderFiles,
		std::vector<VkShaderStageFlagBits> shaderStageFlags,
		VkPipelineLayout* piLayout, VkPipeline* pi,
		VkPipelineVertexInputStateCreateInfo* vertexInputSCI,
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
		/* Color Blend*/
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		std::vector<pipelines::MgPipelineEx> extends{};
		extends.push_back(pipelines::MgPipelineEx::Counter_Clock_Wise);
		/* һЩ���⴦���߼� */
		switch (kind)
		{
		case pipelines::MgPipelineEx::ShadowMap:
			extends.push_back(kind);
			break;
		}

		mg::pipelines::createPipeline(
			shaderStages.data(), shaderStages.size(),
			&colorBlendAttachment, 1,
			vertexInputSCI,
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
		vkDestroyPipeline(device, pi_shadow_gltf, nullptr);

		vkDestroyPipeline(device, pi_Tex, nullptr);
		vkDestroyPipeline(device, pi_Pbr, nullptr);

		vkDestroyPipelineLayout(device, piLayout_ui, nullptr);
		vkDestroyPipelineLayout(device, piLayout_shadow_h, nullptr);
		vkDestroyPipelineLayout(device, piLayout_shadow, nullptr);			//��Ӱ��Ⱦ �ƹ����
		vkDestroyPipelineLayout(device, piLayout_solid, nullptr);

		vkDestroyDescriptorSetLayout(device, setLayout_ui, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_shadow, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_shadow_h, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_solidTex, nullptr);
	}
};
