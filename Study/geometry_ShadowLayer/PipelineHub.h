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
	VkDescriptorSetLayout setLayout_ubo;	//<ubo>
	VkDescriptorSetLayout setLayout_tex;	//<tex>
	VkDescriptorSetLayout setLayout_ubo_tex;//<ubo,tex>
	/* set-layout ��ϵ� pipe-layout */
	VkPipelineLayout piLayout_ubo_tex;		//��������+ShadowMap
	VkPipelineLayout piLayout_ubo_tex_tex;	//��������+ShadowMap + Object��ͼ /+/  ui����+ui��ͼ+shadow:2d-array
	VkPipelineLayout piLayout_ubo;			//��Ӱ��Ⱦ
	/* ʹ��ͬһ�� pipeline-layout */
	VkPipeline pi_Tex;						//shader::<tex.shader>
	VkPipeline pi_TexArray;					//shader::<texArray.shader>
	VkPipeline pi_TexCube;					//shader::<texCube.shader>
	VkPipeline pi_Tex3D;					//shader::<tex3D.shader>

	VkPipeline pi_ui;						//shader::<ui.shader>
	VkPipeline pi_shadow;					//shader::<shadowCaster.shader>

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
		// <texture>
		types = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		stages = { VK_SHADER_STAGE_FRAGMENT_BIT };
		desCounts = { 1 };
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_tex);
		//shadow+ui
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		stages = { stageVGF , VK_SHADER_STAGE_FRAGMENT_BIT };
		desCounts = { 1,1 };
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_ubo_tex);

		/* Pipeline-Layout */
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		VkPushConstantRange pushRange = { stageVGF,0,constantSize };
		//������Ϣ+Shadow // UI��+UIͼƬ
		VkDescriptorSetLayout setL_view[] = { setLayout_ubo_tex };
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = setL_view;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;		//�����ã��޷��� layout-scene ����
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_ubo_tex));

		//������Ϣ+Shadow  /+/  Object��ͼ       //+//      ui����+ui��ͼ+shadow:2d-array
		VkDescriptorSetLayout setL_solidTex[] = { setLayout_ubo_tex,setLayout_tex };
		pipelineLayoutInfo.setLayoutCount = 2;
		pipelineLayoutInfo.pSetLayouts = setL_solidTex;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_ubo_tex_tex));

		//��Ӱ��Ⱦ::    ���ƹ�ռ�ľ���
		VkDescriptorSetLayout setL_shadow[] = { setLayout_ubo };
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = setL_shadow;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_ubo));

		/* Pipeline-s */
		std::vector<VkShaderStageFlagBits> shaderStages = { VK_SHADER_STAGE_VERTEX_BIT,VK_SHADER_STAGE_FRAGMENT_BIT };
		std::vector<const char*> shaderFiles;
		shaderFiles = { "shaders/tex.vert.spv", "shaders/tex.frag.spv" };
		createPipeline(device, passHub->renderPass, &shaderFiles,shaderStages, &piLayout_ubo_tex_tex, &pi_Tex);
		shaderFiles = { "shaders/tex.vert.spv", "shaders/texArray.frag.spv" };
		createPipeline(device, passHub->renderPass, &shaderFiles,shaderStages, &piLayout_ubo_tex_tex, &pi_TexArray);
		shaderFiles = { "shaders/tex.vert.spv", "shaders/texCube.frag.spv" };
		createPipeline(device, passHub->renderPass, &shaderFiles,shaderStages, &piLayout_ubo_tex_tex, &pi_TexCube);
		shaderFiles = { "shaders/tex3D.vert.spv", "shaders/tex3D.frag.spv" };
		createPipeline(device, passHub->renderPass, &shaderFiles,shaderStages, &piLayout_ubo_tex_tex, &pi_Tex3D);

		shaderFiles = { "shaders/ui.vert.spv","shaders/ui.frag.spv" };
		createPipeline(device, passHub->renderPass, &shaderFiles,shaderStages, &piLayout_ubo_tex_tex, &pi_ui, pipelines::MgPipelineEx::UI);
		/* ��ȾShadowMap�Ĺ��� ʹ�ö��Ƶ�RenderPass */
		shaderStages = { VK_SHADER_STAGE_VERTEX_BIT,VK_SHADER_STAGE_GEOMETRY_BIT };
		shaderFiles = { "shaders/shadowCaster.vert.spv","shaders/shadowCaster.geom.spv"};
		createPipeline(device, passHub->shadowPass, &shaderFiles,shaderStages, &piLayout_ubo, &pi_shadow, pipelines::MgPipelineEx::ShadowMap);
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
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		std::vector<pipelines::MgPipelineEx> extends{};
		/* һЩ���⴦���߼� */
		switch (kind)
		{
			/* UI������ ��Descriptor-Set�У�����vertex */
		case pipelines::MgPipelineEx::UI:
			vertexInputSCI = {};
			vertexInputSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			break;
		case pipelines::MgPipelineEx::ShadowMap:
			extends.push_back(kind);
			break;
		}

		mg::pipelines::createPipeline(
			shaderStages.data(), shaderStages.size(),
			&colorBlendAttachment, 1,
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
		vkDestroyPipeline(device, pi_Tex, nullptr);
		vkDestroyPipeline(device, pi_TexArray, nullptr);
		vkDestroyPipeline(device, pi_TexCube, nullptr);
		vkDestroyPipeline(device, pi_Tex3D, nullptr);
		vkDestroyPipeline(device, pi_ui, nullptr);
		vkDestroyPipeline(device, pi_shadow, nullptr);					//ͼ�ι��� shadow

		vkDestroyPipelineLayout(device, piLayout_ubo_tex_tex, nullptr);
		vkDestroyPipelineLayout(device, piLayout_ubo_tex, nullptr);
		vkDestroyPipelineLayout(device, piLayout_ubo, nullptr);			//��Ӱ��Ⱦ �ƹ����

		vkDestroyDescriptorSetLayout(device, setLayout_ubo, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_tex, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_ubo_tex, nullptr);
	}
};
