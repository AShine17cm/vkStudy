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
	/* 基础的 set-layout */
	VkDescriptorSetLayout setLayout_ui;
	VkDescriptorSetLayout setLayout_shadow;		//阴影渲染 
	VkDescriptorSetLayout setLayout_shadow_h;	//阴影合成 h:holder  管线上的资源继承
	VkDescriptorSetLayout setLayout_pbrBasic;
	VkDescriptorSetLayout setLayout_pbrEnv;
	VkDescriptorSetLayout setLayout_pbrTexs;
	/* set-layout 组合的 pipe-layout */
	VkPipelineLayout piLayout_ui;
	VkPipelineLayout piLayout_shadow;		//阴影渲染
	VkPipelineLayout piLayout_shadow_h;		//阴影合成: h表示占位，用于后续渲染的继承
	VkPipelineLayout piLayout_pbrBasic;		//场景数据+ShadowMap+ PBR + Object 贴图
	VkPipelineLayout piLayout_pbrEnv;
	VkPipelineLayout piLayout_pbrIBL;

	VkPipeline pi_ui;						//shader::<ui.shader>
	VkPipeline pi_shadow_gltf;				//gltf 文件有自己的顶点属性布局
	/* 使用同一个 pipeline-layout */
	VkPipeline pi_pbr_basic;						//shader::<pbr.shader>	用gltf 模型 做PBR 测试
	VkPipeline pi_pbr_IBL;

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
		//阴影合成
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		stages = { stageVGF , VK_SHADER_STAGE_FRAGMENT_BIT };
		desCounts = { 1,1 };
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_shadow_h);
		//pbr
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };
		stages = { VK_SHADER_STAGE_FRAGMENT_BIT  };
		desCounts = { 1};
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_pbrBasic);
		//环境+参数  辉度图  lut  预过滤环境
		types = { 
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
		};
		stages = { 
			VK_SHADER_STAGE_FRAGMENT_BIT ,
			VK_SHADER_STAGE_FRAGMENT_BIT ,
			VK_SHADER_STAGE_FRAGMENT_BIT };
		desCounts = { 1,1,1};
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_pbrEnv);
		//纹理+金属度/高光+法线+OC+自发光 
		types = {
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
		};
		stages = {
			VK_SHADER_STAGE_FRAGMENT_BIT ,
			VK_SHADER_STAGE_FRAGMENT_BIT ,
			VK_SHADER_STAGE_FRAGMENT_BIT ,
			VK_SHADER_STAGE_FRAGMENT_BIT ,
			VK_SHADER_STAGE_FRAGMENT_BIT ,
			VK_SHADER_STAGE_FRAGMENT_BIT };
		desCounts = { 1,1,1,1,1,1 };
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_pbrTexs);
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

		//渲染阶段::  场景信息+Shadow  /+/  PBR  Object贴图
		VkDescriptorSetLayout setL_pbrBasic[] = { setLayout_shadow_h,setLayout_pbrBasic };
		pipelineLayoutInfo.setLayoutCount = 2;
		pipelineLayoutInfo.pSetLayouts = setL_pbrBasic;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_pbrBasic));
		//Pbr  Env
		VkDescriptorSetLayout setL_pbrEnv[] = { setLayout_shadow_h,setLayout_pbrEnv };
		pipelineLayoutInfo.setLayoutCount = 2;
		pipelineLayoutInfo.pSetLayouts = setL_pbrEnv;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_pbrEnv));
		//Pbr  IBL
		VkDescriptorSetLayout setL_pbrTex[] = { setLayout_shadow_h,setLayout_pbrEnv,setLayout_pbrTexs };
		pipelineLayoutInfo.setLayoutCount = 3;
		pipelineLayoutInfo.pSetLayouts = setL_pbrTex;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_pbrIBL));

		/* Pipeline-s */
		std::vector<VkShaderStageFlagBits> shaderStages; 
		std::vector<const char*> shaderFiles;
		/* 顶点布局 UI的数据 在Descriptor-Set中，不走vertex */
		VkPipelineVertexInputStateCreateInfo vertexInputSCI_ui{};
		vertexInputSCI_ui.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		/* 顶点布局 gltf */
		VkVertexInputBindingDescription vertexInputBinding = { 0, sizeof(vkglTF::Model::Vertex), VK_VERTEX_INPUT_RATE_VERTEX };
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
			{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },						//位置
			{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3 },		//法线
			{ 2, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 6 },			//uv 0
			{ 3, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 8 },			//uv 1
			{ 4, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 10 },	//joint 0
			{ 5, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 14 },	//weight 0
			{ 6, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 18 }		//颜色
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
		createPipeline(device, passHub->renderPass, &shaderFiles, shaderStages, &piLayout_ui, &pi_ui, &vertexInputSCI_ui,pipelines::MgPipelineEx::Multisample);
		/* 阴影阶段 使用定制的RenderPass */
		shaderStages = { VK_SHADER_STAGE_VERTEX_BIT,VK_SHADER_STAGE_GEOMETRY_BIT };
		shaderFiles = { "shaders/shadowCaster_gltf.vert.spv","shaders/shadowCaster.geom.spv" };
		createPipeline(device, passHub->shadowPass, &shaderFiles, shaderStages, &piLayout_shadow, &pi_shadow_gltf,&vertexInputSCI_gltf, pipelines::MgPipelineEx::ShadowMap);
		/* pbr */
		shaderStages = { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT };
		shaderFiles = { "shaders/pbr_basic.vert.spv", "shaders/pbr_basic.frag.spv" };
		createPipeline(device, passHub->renderPass, &shaderFiles, shaderStages, &piLayout_pbrBasic, &pi_pbr_basic,&vertexInputSCI_gltf,pipelines::MgPipelineEx::Multisample);
		shaderFiles = { "shaders/pbr_basic.vert.spv", "shaders/pbr_ibl.frag.spv" };
		createPipeline(device, passHub->renderPass, &shaderFiles, shaderStages, &piLayout_pbrIBL, &pi_pbr_IBL, &vertexInputSCI_gltf, pipelines::MgPipelineEx::Multisample);
	}
	/* 根据 shader 创建 pipeline */
	void createPipeline(VkDevice device, VkRenderPass renderPass, 
		std::vector<const char*>* shaderFiles,
		std::vector<VkShaderStageFlagBits> shaderStageFlags,
		VkPipelineLayout* piLayout, VkPipeline* pi,
		VkPipelineVertexInputStateCreateInfo* vertexInputSCI,
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
		/* Color Blend*/
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		std::vector<pipelines::MgPipelineEx> extends{};
		extends.push_back(pipelines::MgPipelineEx::Counter_Clock_Wise);
		/* 一些特殊处理逻辑 */
		switch (kind)
		{
		case pipelines::MgPipelineEx::ShadowMap:
			extends.push_back(kind);
			break;
		case pipelines::MgPipelineEx::Multisample:
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
		vkDestroyPipeline(device, pi_shadow_gltf, nullptr);

		vkDestroyPipeline(device, pi_pbr_basic, nullptr);
		vkDestroyPipeline(device, pi_pbr_IBL, nullptr);

		vkDestroyPipelineLayout(device, piLayout_ui, nullptr);
		vkDestroyPipelineLayout(device, piLayout_shadow_h, nullptr);
		vkDestroyPipelineLayout(device, piLayout_shadow, nullptr);			//阴影渲染 灯光矩阵

		vkDestroyPipelineLayout(device, piLayout_pbrBasic, nullptr);
		vkDestroyPipelineLayout(device, piLayout_pbrEnv, nullptr);
		vkDestroyPipelineLayout(device, piLayout_pbrIBL, nullptr);

		vkDestroyDescriptorSetLayout(device, setLayout_ui, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_shadow, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_shadow_h, nullptr);

		vkDestroyDescriptorSetLayout(device, setLayout_pbrBasic, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_pbrEnv, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_pbrTexs, nullptr);
	}
};
