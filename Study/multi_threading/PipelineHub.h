#pragma once
#include <vector>
#include "vulkan/vulkan.h"
#include "VulkanTools.h"
#include "glm.hpp"
#include "RenderPassHub.h"
using namespace glm;

struct PipelineHub
{
	/* 基础的 set-layout */
	VkDescriptorSetLayout setLayout_ui;
	VkDescriptorSetLayout setLayout_shadow;		//阴影渲染 
	VkDescriptorSetLayout setLayout_shadow_h;	//阴影合成 h:holder  管线上的资源继承
	VkDescriptorSetLayout setLayout_solidTex;	//物体的 albedo
	/* set-layout 组合的 pipe-layout */
	VkPipelineLayout piLayout_ui;
	VkPipelineLayout piLayout_shadow;		//阴影渲染
	VkPipelineLayout piLayout_shadow_h;		//阴影合成: h表示占位，用于后续渲染的继承
	VkPipelineLayout piLayout_solid;		//场景数据+ShadowMap + Object贴图 /+/  ui顶点+ui贴图+shadow:2d-array

	VkPipeline pi_ui;						//shader::<ui.shader>
	VkPipeline pi_shadow;					//shader::<shadowCaster.shader>
	/* 使用同一个 pipeline-layout */
	VkPipeline pi_Tex;						//shader::<tex.shader>
	VkPipeline pi_TexArray;					//shader::<texArray.shader>
	VkPipeline pi_TexCube;					//shader::<texCube.shader>
	VkPipeline pi_Tex3D;					//shader::<tex3D.shader>


	void prepare(VkDevice device, RenderPassHub* passHub, uint32_t constantSize)
	{
		VkShaderStageFlags stageVGF = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		/* Descriptor-Layout */
		std::vector<VkDescriptorType> types;
		std::vector<VkShaderStageFlags> stages;
		std::vector<uint32_t> desCounts;
		/* ui  阴影展示 */
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
		/* 阴影渲染 */
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };
		stages = { stageVGF };
		desCounts = { 1 };
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_shadow);
		// <texture>
		types = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		stages = { VK_SHADER_STAGE_FRAGMENT_BIT };
		desCounts = { 1 };
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_solidTex);
		//阴影合成
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		stages = { stageVGF , VK_SHADER_STAGE_FRAGMENT_BIT };
		desCounts = { 1,1 };
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_shadow_h);

		/* Pipeline-Layout */
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		VkPushConstantRange pushRange = { stageVGF,0,constantSize };
		//ui 阴影展示
		VkDescriptorSetLayout setL_ui[] = { setLayout_ui };
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = setL_ui;
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_ui));
		//阴影渲染::    到灯光空间的矩阵
		VkDescriptorSetLayout setL_shadow[] = { setLayout_shadow };
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = setL_shadow;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_shadow));

		//阴影合成::  用于pipeline-layout 的资源继承
		VkDescriptorSetLayout setL_shadow_holder[] = { setLayout_shadow_h };
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = setL_shadow_holder;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;		//不设置，无法将 layout-scene 复用
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_shadow_h));

		//渲染阶段::  场景信息+Shadow  /+/  Object贴图
		VkDescriptorSetLayout setL_solidTex[] = { setLayout_shadow_h,setLayout_solidTex };
		pipelineLayoutInfo.setLayoutCount = 2;
		pipelineLayoutInfo.pSetLayouts = setL_solidTex;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_solid));

		/* Pipeline-s */
		std::vector<VkShaderStageFlagBits> shaderStages; 
		std::vector<const char*> shaderFiles;
		//ui
		shaderStages= { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT };
		shaderFiles = { "shaders/ui.vert.spv","shaders/ui.frag.spv" };
		createPipeline(device, passHub->renderPass, &shaderFiles, shaderStages, &piLayout_ui, &pi_ui, pipelines::MgPipelineEx::UI);
		/* 阴影阶段 使用定制的RenderPass */
		shaderStages = { VK_SHADER_STAGE_VERTEX_BIT,VK_SHADER_STAGE_GEOMETRY_BIT };
		shaderFiles = { "shaders/shadowCaster.vert.spv","shaders/shadowCaster.geom.spv" };
		createPipeline(device, passHub->shadowPass, &shaderFiles, shaderStages, &piLayout_shadow, &pi_shadow, pipelines::MgPipelineEx::ShadowMap);
		/* 渲染场景阶段 */
		shaderStages = { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT };
		shaderFiles = { "shaders/tex.vert.spv", "shaders/tex.frag.spv" };
		createPipeline(device, passHub->renderPass, &shaderFiles,shaderStages, &piLayout_solid, &pi_Tex);
		shaderFiles = { "shaders/tex.vert.spv", "shaders/texArray.frag.spv" };
		createPipeline(device, passHub->renderPass, &shaderFiles,shaderStages, &piLayout_solid, &pi_TexArray);
		shaderFiles = { "shaders/tex.vert.spv", "shaders/texCube.frag.spv" };
		createPipeline(device, passHub->renderPass, &shaderFiles,shaderStages, &piLayout_solid, &pi_TexCube);
		shaderFiles = { "shaders/tex3D.vert.spv", "shaders/tex3D.frag.spv" };
		createPipeline(device, passHub->renderPass, &shaderFiles,shaderStages, &piLayout_solid, &pi_Tex3D);
	}
	/* 根据 shader 创建 pipeline */
	void createPipeline(VkDevice device, VkRenderPass renderPass, 
		std::vector<const char*>* shaderFiles,
		std::vector<VkShaderStageFlagBits> shaderStageFlags,
		VkPipelineLayout* piLayout, VkPipeline* pi,
		pipelines::MgPipelineEx kind = pipelines::MgPipelineEx::None)
	{
		std::vector<VkShaderModule> shaderModules;
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		//从文件中 加载 spv形式的 shader
		for (uint32_t i = 0; i < (*shaderFiles).size(); i++)
		{
			VkShaderModule module = mg::shaders::loadShader((*shaderFiles)[i], device);
			VkPipelineShaderStageCreateInfo shaderStage = {};
			shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStage.stage = shaderStageFlags[i];
			shaderStage.module = module;
			shaderStage.pName = "main";
			assert(shaderStage.module != VK_NULL_HANDLE);

			shaderModules.push_back(module); //存储到 类中
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
		/* 一些特殊处理逻辑 */
		switch (kind)
		{
			/* UI的数据 在Descriptor-Set中，不走vertex */
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

		//创建pipeline完毕，销毁shader-module
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
		vkDestroyPipeline(device, pi_shadow, nullptr);					//图形管线 shadow

		vkDestroyPipeline(device, pi_Tex, nullptr);
		vkDestroyPipeline(device, pi_TexArray, nullptr);
		vkDestroyPipeline(device, pi_TexCube, nullptr);
		vkDestroyPipeline(device, pi_Tex3D, nullptr);

		vkDestroyPipelineLayout(device, piLayout_ui, nullptr);
		vkDestroyPipelineLayout(device, piLayout_shadow_h, nullptr);
		vkDestroyPipelineLayout(device, piLayout_shadow, nullptr);			//阴影渲染 灯光矩阵
		vkDestroyPipelineLayout(device, piLayout_solid, nullptr);

		vkDestroyDescriptorSetLayout(device, setLayout_ui, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_shadow, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_shadow_h, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_solidTex, nullptr);
	}
};
