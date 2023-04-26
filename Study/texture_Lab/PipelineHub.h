#pragma once
#include <vector>
#include "vulkan/vulkan.h"
#include "VulkanTools.h"
#include "glm.hpp"
using namespace glm;

struct PipelineHub
{
	/* 基础的 set-layout */
	VkDescriptorSetLayout setLayout_ubo;	//<ubo>
	VkDescriptorSetLayout setLayout_tex;	//<tex>
	VkDescriptorSetLayout setLayout_ui;		//<ubo,tex>
	/* set-layout 组合的 pipe-layout */
	VkPipelineLayout piLayout_ui;			//<ubo,tex>
	VkPipelineLayout piLayout_view;			//<ubo> camera-light
	VkPipelineLayout piLayout_tex;			//<ubo,tex>
	VkPipelineLayout piLayout_instance;		//<ubo,ubo,tex>
	/* 使用同一个 pipeline-layout */
	VkPipeline pi_Tex;						//shader::<tex.vert tex.frag>
	VkPipeline pi_TexArray;					//shader::<texArray.vert texArray.frag>
	VkPipeline pi_TexCube;					//shader::<texCube.vert texCube.frag>
	VkPipeline pi_Tex3D;					//shader::<tex3D.vert tex3D.frag>

	VkPipeline pi_ui;						//shader::<ui.vert ui.frag>
	VkPipeline pi_Instance;					//shader::<instancing.vert instancing.frag>

	void prepare(VkDevice device, VkRenderPass renderPass,uint32_t constantSize)
	{
		/* Descriptor-Layout */
		std::vector<VkDescriptorType> types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };
		std::vector<VkShaderStageFlags> stages = { VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT };
		std::vector<uint32_t> desCounts = { 1 };
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_ubo);
		// <texture>
		types = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		stages = { VK_SHADER_STAGE_FRAGMENT_BIT };
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_tex);
		//ui
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		stages= { VK_SHADER_STAGE_VERTEX_BIT , VK_SHADER_STAGE_FRAGMENT_BIT };
		desCounts = { 1,1 };
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_ui);

		/* Pipeline-Layout */
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		VkPushConstantRange pushRange = { VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,0,constantSize };
		//View
		VkDescriptorSetLayout setL_view[] = { setLayout_ubo };
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = setL_view;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;		//不设置，无法将 layout-scene 复用
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_view));
		//ui
		VkDescriptorSetLayout setL_ui[] = { setLayout_ui };
		pipelineLayoutInfo.pSetLayouts = setL_ui;
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_ui));

		//Solid-Tex
		VkDescriptorSetLayout setL_solidTex[] = { setLayout_ubo,setLayout_tex };
		pipelineLayoutInfo.setLayoutCount = 2;
		pipelineLayoutInfo.pSetLayouts = setL_solidTex;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_tex));

		//Instancing
		VkDescriptorSetLayout setL_Instance[] = { setLayout_ubo,setLayout_ubo,setLayout_tex };
		pipelineLayoutInfo.setLayoutCount = 3;
		pipelineLayoutInfo.pSetLayouts = setL_Instance;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;

		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_instance));

		/* Pipeline-s */
		std::vector<const char*> shaderFiles = { "shaders/tex.vert.spv", "shaders/tex.frag.spv" };
		createPipeline(device, renderPass, &shaderFiles, &piLayout_tex, &pi_Tex, 0);
		shaderFiles = { "shaders/texArray.vert.spv", "shaders/texArray.frag.spv" };
		createPipeline(device, renderPass, &shaderFiles, &piLayout_tex, &pi_TexArray, 0);
		shaderFiles = { "shaders/texCube.vert.spv", "shaders/texCube.frag.spv" };
		createPipeline(device, renderPass, &shaderFiles, &piLayout_tex, &pi_TexCube, 0);
		shaderFiles = { "shaders/tex3D.vert.spv", "shaders/tex3D.frag.spv" };
		createPipeline(device, renderPass, &shaderFiles, &piLayout_tex, &pi_Tex3D, 0);

		shaderFiles = { "shaders/instancing.vert.spv", "shaders/instancing.frag.spv" };
		createPipeline(device, renderPass, &shaderFiles, &piLayout_instance, &pi_Instance,0);
		shaderFiles = { "shaders/ui.vert.spv","shaders/ui.frag.spv" };
		createPipeline(device, renderPass, &shaderFiles, &piLayout_ui, &pi_ui,1);
	}
	/* 根据 shader 创建 pipeline */
	void createPipeline(VkDevice device, VkRenderPass renderPass, std::vector<const char*>* shaderFiles, VkPipelineLayout* piLayout, VkPipeline* pi,int kind)
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
		/* UI的数据 在Descriptor-Set中，不走vertex */
		if(kind==0) vertexInputSCI= geos::Geo::getVertexInput(&attributes, &bindings);

		/* Color Blend*/
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		mg::pipelines::createPipeline(
			shaderStages.data(), shaderStages.size(),
			&colorBlendAttachment, 1,
			&vertexInputSCI,
			*piLayout,
			renderPass,
			device,
			nullptr,
			pi,
			nullptr, nullptr, 0);

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

		vkDestroyPipelineLayout(device, piLayout_ui, nullptr);
		vkDestroyPipelineLayout(device, piLayout_instance, nullptr);
		vkDestroyPipelineLayout(device, piLayout_tex, nullptr);
		vkDestroyPipelineLayout(device, piLayout_view, nullptr);

		vkDestroyDescriptorSetLayout(device, setLayout_ubo, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_tex, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_ui, nullptr);
	}
};
