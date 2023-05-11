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
	VkDescriptorSetLayout setLayout_ubo;				//阴影阶段
	VkDescriptorSetLayout setLayout_solid;				//geo_mrt阶段: <normal,tex>
	VkDescriptorSetLayout setLayout_deferred_compose;	//compose阶段 shadow,pos,norm,albedo 4个tex + 一个调试的ubo
	/* set-layout 组合的 pipe-layout */
	VkPipelineLayout piLayout_ui;
	VkPipelineLayout piLayout_ubo;				//阴影渲染
	VkPipelineLayout piLayout_geo_mrt_solid;	//geo-mrt
	VkPipelineLayout piLayout_deferred_compose;	//延迟渲染 最终着色
	/* 使用同一个 pipeline-layout */
	VkPipeline pi_ui;						//shader::<ui.shader>
	VkPipeline pi_shadow;					//shader::<shadowCaster.shader>

	VkPipeline pi_geo_mrt_tex;				//shader::<tex.shader>
	VkPipeline pi_geo_mrt_texArray;			//shader::<texArray.shader>
	VkPipeline pi_geo_mrt_cubeMap;			//shader::<texCube.shader>

	VkPipeline pi_deferred_compose;			//合成着色的管线	

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
		//ui顶点+贴图
		VkDescriptorSetLayout setL_uiTex[] = { setLayout_ui };
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = setL_uiTex;
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_ui));
		//阴影渲染::    到灯光空间的矩阵
		VkDescriptorSetLayout setL_shadow[] = { setLayout_ubo };
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = setL_shadow;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_ubo));

		//geo_mrt阶段:: 场景ubo  /+/  normal+tex      
		VkDescriptorSetLayout setL_SolidTex[] = { setLayout_ubo,setLayout_solid };
		pipelineLayoutInfo.setLayoutCount = 2;
		pipelineLayoutInfo.pSetLayouts = setL_SolidTex;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_geo_mrt_solid));
		//compose阶段
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
		/* 阴影阶段 geometry-shader 多光源阴影 */
		shaderStages = { VK_SHADER_STAGE_VERTEX_BIT,VK_SHADER_STAGE_GEOMETRY_BIT };
		shaderFiles = { "shaders/shadowCaster.vert.spv","shaders/shadowCaster.geom.spv" };
		createPipeline(device, passHub->shadowPass, &shaderFiles, shaderStages, &piLayout_ubo, &pi_shadow, pipelines::MgPipelineEx::ShadowMap);

		/* geo_mrt 阶段 */
		shaderStages= { VK_SHADER_STAGE_VERTEX_BIT,VK_SHADER_STAGE_FRAGMENT_BIT };
		shaderFiles = { "shaders/geo_mrt.vert.spv", "shaders/geo_mrt_tex.frag.spv" };
		createPipeline(device, passHub->geo_mrt_Pass, &shaderFiles, shaderStages, &piLayout_geo_mrt_solid, &pi_geo_mrt_tex, pipelines::MgPipelineEx::Geo_MRT);
		shaderFiles = { "shaders/geo_mrt.vert.spv", "shaders/geo_mrt_texArray.frag.spv" };
		createPipeline(device, passHub->geo_mrt_Pass, &shaderFiles, shaderStages, &piLayout_geo_mrt_solid, &pi_geo_mrt_texArray, pipelines::MgPipelineEx::Geo_MRT);
		shaderFiles = { "shaders/geo_mrt.vert.spv", "shaders/geo_mrt_cubeMap.frag.spv" };
		createPipeline(device, passHub->geo_mrt_Pass, &shaderFiles, shaderStages, &piLayout_geo_mrt_solid, &pi_geo_mrt_cubeMap, pipelines::MgPipelineEx::Geo_MRT);
		/* compose阶段 */
		shaderStages = { VK_SHADER_STAGE_VERTEX_BIT ,VK_SHADER_STAGE_FRAGMENT_BIT };
		shaderFiles = { "shaders/deferred_compose.vert.spv","shaders/deferred_compose.frag.spv" };
		createPipeline(device, passHub->deferredPass, &shaderFiles, shaderStages, &piLayout_deferred_compose, &pi_deferred_compose, pipelines::MgPipelineEx::Deferred_Compose);
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
		/* 一些特殊处理逻辑 */
		switch (kind)
		{
			/* UI的数据 在Descriptor-Set中，不走vertex */
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
		vkDestroyPipeline(device, pi_geo_mrt_tex, nullptr);
		vkDestroyPipeline(device, pi_geo_mrt_texArray, nullptr);
		vkDestroyPipeline(device, pi_geo_mrt_cubeMap, nullptr);
		vkDestroyPipeline(device, pi_deferred_compose, nullptr);

		vkDestroyPipelineLayout(device, piLayout_ui, nullptr);
		vkDestroyPipelineLayout(device, piLayout_ubo, nullptr);			//阴影渲染 灯光矩阵
		vkDestroyPipelineLayout(device, piLayout_geo_mrt_solid, nullptr);
		vkDestroyPipelineLayout(device, piLayout_deferred_compose, nullptr);

		vkDestroyDescriptorSetLayout(device, setLayout_ui, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_ubo, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_solid, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_deferred_compose, nullptr);
	}
};
