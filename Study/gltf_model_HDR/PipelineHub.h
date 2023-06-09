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
	VkDescriptorSetLayout setLayout_shadow;		//阴影渲染 
	VkDescriptorSetLayout setLayout_shadow_h;	//阴影合成 h:holder  管线上的资源继承
	VkDescriptorSetLayout setLayout_pbrBasic;
	VkDescriptorSetLayout setLayout_pbrEnv;
	VkDescriptorSetLayout setLayout_pbrTexs;

	VkDescriptorSetLayout setLayout_hdr_offscreen;
	VkDescriptorSetLayout setLayout_hdr_bloom;
	VkDescriptorSetLayout setLayout_ldr;
	VkDescriptorSetLayout setLayout_blend;
	/* set-layout 组合的 pipe-layout */
	VkPipelineLayout piLayout_shadow;		//阴影渲染
	VkPipelineLayout piLayout_shadow_h;		//阴影合成: h表示占位，用于后续渲染的继承
	VkPipelineLayout piLayout_pbrBasic;		//场景数据+ShadowMap+ PBR + Object 贴图
	VkPipelineLayout piLayout_pbrEnv;
	VkPipelineLayout piLayout_pbrIBL;

	VkPipelineLayout piLayout_hdr_offscreen;
	VkPipelineLayout piLayout_hdr_bloom;
	VkPipelineLayout piLayout_ldr;
	VkPipelineLayout piLayout_blend;

	VkPipeline pi_shadow_gltf;				//gltf 文件有自己的顶点属性布局
	/* 使用同一个 pipeline-layout */
	VkPipeline pi_pbr_basic;						//shader::<pbr.shader>	用gltf 模型 做PBR 测试
	VkPipeline pi_pbr_IBL;

	VkPipeline pi_hdr_offscreen;
	VkPipeline pi_hdr_bloom;
	VkPipeline pi_ldr;
	VkPipeline pi_blend;

	void prepare(VkDevice device, RenderPassHub* passHub, uint32_t constantSize,uint32_t hdr_constant)
	{
		VkShaderStageFlags stageVGF = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		/* Descriptor-Layout */
		std::vector<VkDescriptorType> types;
		std::vector<VkShaderStageFlags> stages;
		std::vector<uint32_t> desCounts;
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
		//HDR  高动态 映射 地动态,高亮分离
		types = {
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
		};
		stages = {
			VK_SHADER_STAGE_FRAGMENT_BIT 
		};
		desCounts = { 1 };
		//HDR Bloom
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_hdr_offscreen);
		//Blend
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_blend);

		types = {
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
		};
		stages = {
			VK_SHADER_STAGE_FRAGMENT_BIT,
			VK_SHADER_STAGE_FRAGMENT_BIT
		};
		desCounts = { 1,1 };
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_hdr_bloom);
		//Ldr
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_ldr);

		/* Pipeline-Layout */
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		VkPushConstantRange pushRange = { stageVGF,0,constantSize };
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
		//Hdr Offscreen
		VkPushConstantRange pushRange_hdr = { VK_SHADER_STAGE_FRAGMENT_BIT,0,hdr_constant };//曝光度
		VkDescriptorSetLayout setL_hdrTex[] = { setLayout_hdr_offscreen };
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = setL_hdrTex;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange_hdr;
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_hdr_offscreen));
		//Hdr Bloom
		VkDescriptorSetLayout setL_hdrBloom[] = { setLayout_hdr_bloom };
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = setL_hdrBloom;
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_hdr_bloom));
		//LDR
		VkDescriptorSetLayout setL_ldr[] = { setLayout_ldr };
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = setL_ldr;
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_ldr));
		//Blend
		VkDescriptorSetLayout setL_blend[] = { setLayout_blend };
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = setL_blend;
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_blend));

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

		/* 阴影阶段 使用定制的RenderPass */
		shaderStages = { VK_SHADER_STAGE_VERTEX_BIT,VK_SHADER_STAGE_GEOMETRY_BIT };
		shaderFiles = { "shaders/shadowCaster_gltf.vert.spv","shaders/shadowCaster.geom.spv" };
		createPipeline(device, passHub->shadowPass, &shaderFiles, shaderStages, &piLayout_shadow, &pi_shadow_gltf,&vertexInputSCI_gltf, pipelines::MgPipelineEx::ShadowMap);
		/* pbr */
		shaderStages = { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT };
		shaderFiles = { "shaders/pbr_basic.vert.spv", "shaders/pbr_basic.frag.spv" };
		createPipeline(device, passHub->msaaTarget.renderPass, &shaderFiles, shaderStages, &piLayout_pbrBasic, &pi_pbr_basic,&vertexInputSCI_gltf,pipelines::MgPipelineEx::Multisample);
		shaderFiles = { "shaders/pbr_basic.vert.spv", "shaders/pbr_ibl.frag.spv" };
		createPipeline(device, passHub->msaaTarget.renderPass, &shaderFiles, shaderStages, &piLayout_pbrIBL, &pi_pbr_IBL, &vertexInputSCI_gltf, pipelines::MgPipelineEx::Multisample);
		/* hdr */
		shaderStages = { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT };
		shaderFiles = { "shaders/screen.vert.spv", "shaders/hdr_offscreen.frag.spv" };
		createPipeline(device, passHub->offscreen.renderPass, &shaderFiles, shaderStages, &piLayout_hdr_offscreen, &pi_hdr_offscreen, &vertexInputSCI_ui,pipelines::MgPipelineEx::Hdr_Offscreen);
		//需要 开启Blend
		shaderFiles = { "shaders/screen.vert.spv", "shaders/hdr_bloom.frag.spv" };
		createPipeline(device, passHub->bloomPass.renderPass, &shaderFiles, shaderStages, &piLayout_hdr_bloom, &pi_hdr_bloom, &vertexInputSCI_ui,pipelines::MgPipelineEx::Bloom_Blend);
		/* ldr */
		shaderFiles = { "shaders/screen.vert.spv", "shaders/ldr.frag.spv" };
		createPipeline(device, passHub->renderPass, &shaderFiles, shaderStages, &piLayout_ldr, &pi_ldr, &vertexInputSCI_ui);
		/* blend */
		shaderFiles = { "shaders/screen.vert.spv", "shaders/blend.frag.spv" };
		createPipeline(device, passHub->renderPass, &shaderFiles, shaderStages, &piLayout_blend, &pi_blend, &vertexInputSCI_ui, pipelines::MgPipelineEx::Bloom_Blend);
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
		std::array<VkPipelineColorBlendAttachmentState, 2> colorBlends;
		//VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlends[0] = {};
		colorBlends[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlends[0].blendEnable = VK_FALSE;
		colorBlends[1] = {};
		colorBlends[1].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlends[1].blendEnable = VK_FALSE;

		uint32_t countBlend = 1;
		if (pipelines::MgPipelineEx::Hdr_Offscreen == kind)
		{
			countBlend = 2;
		}
		//Bloom 混合到帧中
		if (pipelines::MgPipelineEx::Bloom_Blend==kind)
		{
			colorBlends[0].colorWriteMask = 0xF;
			colorBlends[0].blendEnable = VK_TRUE;
			colorBlends[0].colorBlendOp = VK_BLEND_OP_ADD;
			colorBlends[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlends[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlends[0].alphaBlendOp = VK_BLEND_OP_ADD;
			colorBlends[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			colorBlends[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
		}

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
			colorBlends.data(), countBlend,
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
		vkDestroyPipeline(device, pi_shadow_gltf, nullptr);

		vkDestroyPipeline(device, pi_pbr_basic, nullptr);
		vkDestroyPipeline(device, pi_pbr_IBL, nullptr);

		vkDestroyPipeline(device, pi_hdr_offscreen, nullptr);
		vkDestroyPipeline(device, pi_hdr_bloom, nullptr);
		vkDestroyPipeline(device, pi_ldr, nullptr);
		vkDestroyPipeline(device, pi_blend, nullptr);

		vkDestroyPipelineLayout(device, piLayout_shadow_h, nullptr);
		vkDestroyPipelineLayout(device, piLayout_shadow, nullptr);			//阴影渲染 灯光矩阵

		vkDestroyPipelineLayout(device, piLayout_pbrBasic, nullptr);
		vkDestroyPipelineLayout(device, piLayout_pbrEnv, nullptr);
		vkDestroyPipelineLayout(device, piLayout_pbrIBL, nullptr);

		vkDestroyPipelineLayout(device, piLayout_hdr_offscreen, nullptr);
		vkDestroyPipelineLayout(device, piLayout_hdr_bloom, nullptr);
		vkDestroyPipelineLayout(device, piLayout_ldr, nullptr);
		vkDestroyPipelineLayout(device, piLayout_blend, nullptr);

		vkDestroyDescriptorSetLayout(device, setLayout_shadow, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_shadow_h, nullptr);

		vkDestroyDescriptorSetLayout(device, setLayout_pbrBasic, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_pbrEnv, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_pbrTexs, nullptr);

		vkDestroyDescriptorSetLayout(device, setLayout_hdr_offscreen, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_hdr_bloom, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_ldr, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_blend, nullptr);
	}
};
