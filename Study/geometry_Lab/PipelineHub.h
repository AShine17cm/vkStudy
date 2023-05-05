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
	VkDescriptorSetLayout setLayout_ubo;	//<ubo>
	VkDescriptorSetLayout setLayout_ubo_geo;//<ubo> SHADER_STAGE_GEOMETRY 
	VkDescriptorSetLayout setLayout_ubo_ubo_geo;
	VkDescriptorSetLayout setLayout_tex;	//<tex>
	VkDescriptorSetLayout setLayout_ubo_tex;//<ubo,tex>
	/* set-layout 组合的 pipe-layout */
	VkPipelineLayout piLayout_ubo_tex;		//场景数据+ShadowMap
	VkPipelineLayout piLayout_ubo_tex_tex;	//场景数据+ShadowMap + Object贴图
	VkPipelineLayout piLayout_instance;		//场景数据+ShadowMap + 实例数据+Object贴图
	VkPipelineLayout piLayout_ubo;			//阴影渲染
	VkPipelineLayout piLayout_ubo_ubo;		//阴影渲染 实例化
	VkPipelineLayout piLayout_geo_ubo;		//geo-debug
	VkPipelineLayout piLayout_geo_ubo_instancing;
	/* 使用同一个 pipeline-layout */
	VkPipeline pi_Tex;						//shader::<tex.shader>
	VkPipeline pi_TexArray;					//shader::<texArray.shader>
	VkPipeline pi_TexCube;					//shader::<texCube.shader>
	VkPipeline pi_Tex3D;					//shader::<tex3D.shader>

	VkPipeline pi_ui;						//shader::<ui.shader>
	VkPipeline pi_Instance;					//shader::<instancing.shader>
	VkPipeline pi_shadow;					//shader::<shadowCaster.shader>
	VkPipeline pi_shadow_instancing;		//shader::<shadowCasterInstancing.vert shadowCaster.frag>
	VkPipeline pi_geo_debug;				//shader::<geo_debug.geom>
	VkPipeline pi_geo_debug_instancing;		//shader::<geo_debug_instancing.geom>

	void prepare(VkDevice device,RenderPassHub* passHub, uint32_t constantSize)
	{
		/* Descriptor-Layout */
		std::vector<VkDescriptorType> types;
		std::vector<VkShaderStageFlags> stages;
		std::vector<uint32_t> desCounts;
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };
		stages = { VK_SHADER_STAGE_VERTEX_BIT };
		desCounts = { 1 };
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_ubo);
		//<texture>
		types = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		stages = { VK_SHADER_STAGE_FRAGMENT_BIT };
		desCounts = { 1 };
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_tex);
		//shadow+ui
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		stages = { VK_SHADER_STAGE_VERTEX_BIT , VK_SHADER_STAGE_FRAGMENT_BIT };
		desCounts = { 1,1 };
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_ubo_tex);
		//geometry阶段使用的 UBO
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };
		stages = { VK_SHADER_STAGE_GEOMETRY_BIT };
		desCounts = { 1 };
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_ubo_geo);
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };
		stages = { VK_SHADER_STAGE_VERTEX_BIT,VK_SHADER_STAGE_GEOMETRY_BIT };
		desCounts = { 1,1 };
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_ubo_ubo_geo);

		/* Pipeline-Layout */
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		VkPushConstantRange pushRange = { VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT|VK_SHADER_STAGE_GEOMETRY_BIT,0,constantSize };
		//场景信息+Shadow // UI点+UI图片
		VkDescriptorSetLayout setL_view[] = { setLayout_ubo_tex };
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = setL_view;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;		//不设置，无法将 layout-scene 复用
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_ubo_tex));

		//场景信息+Shadow  /+/  Object贴图
		VkDescriptorSetLayout setL_solidTex[] = { setLayout_ubo_tex,setLayout_tex };
		pipelineLayoutInfo.setLayoutCount = 2;
		pipelineLayoutInfo.pSetLayouts = setL_solidTex;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_ubo_tex_tex));

		//Instancing::   场景信息+Shadow  /+/  实例数据+Object贴图
		VkDescriptorSetLayout setL_Instance[] = { setLayout_ubo_tex,setLayout_ubo_tex };
		pipelineLayoutInfo.setLayoutCount = 2;
		pipelineLayoutInfo.pSetLayouts = setL_Instance;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_instance));
		//阴影渲染::    到灯光空间的矩阵
		VkDescriptorSetLayout setL_shadow[] = { setLayout_ubo };
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = setL_shadow;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_ubo));

		VkDescriptorSetLayout setL_shadow_instancing[] = { setLayout_ubo,setLayout_ubo };
		pipelineLayoutInfo.setLayoutCount = 2;
		pipelineLayoutInfo.pSetLayouts = setL_shadow_instancing;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_ubo_ubo));
		//Geometry 调试 Normal
		VkDescriptorSetLayout setL_geo_debug[] = { setLayout_ubo_geo };
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = setL_geo_debug;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_geo_ubo));
		VkDescriptorSetLayout setL_geo_debug_instancing[] = { setLayout_ubo_ubo_geo };
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = setL_geo_debug_instancing;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_geo_ubo_instancing));

		/* Pipeline-s */
		std::vector<const char*> shaderFiles;
		shaderFiles = { "shaders/tex.vert.spv", "shaders/tex.frag.spv" };
		createPipeline(device, passHub->renderPass, &shaderFiles, &piLayout_ubo_tex_tex, &pi_Tex);
		shaderFiles = { "shaders/texArray.vert.spv", "shaders/texArray.frag.spv" };
		createPipeline(device, passHub->renderPass, &shaderFiles, &piLayout_ubo_tex_tex, &pi_TexArray);
		shaderFiles = { "shaders/texCube.vert.spv", "shaders/texCube.frag.spv" };
		createPipeline(device, passHub->renderPass, &shaderFiles, &piLayout_ubo_tex_tex, &pi_TexCube);
		shaderFiles = { "shaders/tex3D.vert.spv", "shaders/tex3D.frag.spv" };
		createPipeline(device, passHub->renderPass, &shaderFiles, &piLayout_ubo_tex_tex, &pi_Tex3D);

		shaderFiles = { "shaders/instancing.vert.spv", "shaders/instancing.frag.spv" };
		createPipeline(device, passHub->renderPass, &shaderFiles, &piLayout_instance, &pi_Instance);
		shaderFiles = { "shaders/ui.vert.spv","shaders/ui.frag.spv" };
		createPipeline(device, passHub->renderPass, &shaderFiles, &piLayout_ubo_tex, &pi_ui,pipelines::MgPipelineEx::UI );
		/* 渲染ShadowMap的管线 使用定制的RenderPass */
		shaderFiles = { "shaders/shadowCaster.vert.spv","shaders/shadowCaster.frag.spv" };
		createPipeline(device, passHub->shadowPass, &shaderFiles, &piLayout_ubo, &pi_shadow,pipelines::MgPipelineEx::ShadowMap);
		shaderFiles = { "shaders/shadowCasterInstancing.vert.spv","shaders/shadowCaster.frag.spv" };
		createPipeline(device, passHub->shadowPass, &shaderFiles, &piLayout_ubo_ubo, &pi_shadow_instancing,pipelines::MgPipelineEx::ShadowMap);
		/* geometry-shader */
		shaderFiles = { "shaders/geo_debug.vert.spv","shaders/geo_debug.geom.spv","shaders/geo_debug.frag.spv" };
		createPipeline_Geo(device, passHub->renderPass, &shaderFiles, &piLayout_geo_ubo, &pi_geo_debug);
		shaderFiles = { "shaders/geo_debug_instancing.vert.spv","shaders/geo_debug_instancing.geom.spv","shaders/geo_debug.frag.spv" };
		createPipeline_Geo(device, passHub->renderPass, &shaderFiles, &piLayout_geo_ubo_instancing, &pi_geo_debug_instancing);
	}
	/* 根据 shader 创建 pipeline */
	void createPipeline(VkDevice device, VkRenderPass renderPass, std::vector<const char*>* shaderFiles, VkPipelineLayout* piLayout, VkPipeline* pi,
		pipelines::MgPipelineEx kind=pipelines::MgPipelineEx::None)
	{
		std::vector<VkShaderStageFlagBits> shaderStageFlags = { VK_SHADER_STAGE_VERTEX_BIT,VK_SHADER_STAGE_FRAGMENT_BIT };
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
	/* 包含 Geometry-Shader 的Pipeline */
	void createPipeline_Geo(VkDevice device, VkRenderPass renderPass, std::vector<const char*>* shaderFiles, VkPipelineLayout* piLayout, VkPipeline* pi)
	{
		std::vector<VkShaderStageFlagBits> shaderStageFlags = { VK_SHADER_STAGE_VERTEX_BIT,VK_SHADER_STAGE_GEOMETRY_BIT, VK_SHADER_STAGE_FRAGMENT_BIT };
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
		vkDestroyPipeline(device, pi_Tex, nullptr);
		vkDestroyPipeline(device, pi_TexArray, nullptr);
		vkDestroyPipeline(device, pi_TexCube, nullptr);
		vkDestroyPipeline(device, pi_Tex3D, nullptr);
		vkDestroyPipeline(device, pi_ui, nullptr);
		vkDestroyPipeline(device, pi_Instance, nullptr);
		vkDestroyPipeline(device, pi_shadow, nullptr);					//图形管线 shadow
		vkDestroyPipeline(device, pi_shadow_instancing, nullptr);		//图形管线 shadow-instancing
		vkDestroyPipeline(device, pi_geo_debug, nullptr);				//图形管线 normal调试:geometry-shader
		vkDestroyPipeline(device, pi_geo_debug_instancing, nullptr);	//图形管线 normal调试

		vkDestroyPipelineLayout(device, piLayout_instance, nullptr);
		vkDestroyPipelineLayout(device, piLayout_ubo_tex_tex, nullptr);
		vkDestroyPipelineLayout(device, piLayout_ubo_tex, nullptr);
		vkDestroyPipelineLayout(device, piLayout_ubo, nullptr);			//阴影渲染 灯光矩阵
		vkDestroyPipelineLayout(device, piLayout_ubo_ubo, nullptr);		//阴影渲染 灯光矩阵+实例化数据
		vkDestroyPipelineLayout(device, piLayout_geo_ubo, nullptr);		//geo-debug
		vkDestroyPipelineLayout(device, piLayout_geo_ubo_instancing, nullptr);

		vkDestroyDescriptorSetLayout(device, setLayout_ubo, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_ubo_geo, nullptr);	//geo-debug
		vkDestroyDescriptorSetLayout(device, setLayout_ubo_ubo_geo, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_tex, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_ubo_tex, nullptr);
	}
};
