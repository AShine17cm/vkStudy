#pragma once
#include <vector>
#include "vulkan/vulkan.h"
#include "VulkanTools.h"
#include "glm.hpp"
using namespace glm;

struct PipelineHub
{
	VkDescriptorSetLayout setLayout_ubo;
	VkDescriptorSetLayout setLayout_tex;

	VkPipelineLayout piLayout_scene;
	VkPipelineLayout piLayout_solidTex;
	VkPipelineLayout piLayout_instance;

	VkPipeline pi_SolidTex;
	VkPipeline pi_Instance;

	void prepare(VkDevice device, VkRenderPass renderPass)
	{
		/* Descriptor-Layout */
		//UBO <view-proj-light-camera> 或者 <InstanceData>
		std::vector<VkDescriptorType> types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };
		std::vector<VkShaderStageFlags> stages = { VK_SHADER_STAGE_VERTEX_BIT| VK_SHADER_STAGE_FRAGMENT_BIT };
		std::vector<uint32_t> desCounts = { 1};
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_ubo);
		
		// <texture>
		types = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		stages = { VK_SHADER_STAGE_FRAGMENT_BIT };
		desCounts = { 1 };
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_tex);

		/* binding = push_constant	<model> */
		VkPushConstantRange pushRange = { VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT,0,sizeof(mat4) };

		/* Pipeline-Layout */
		//Scene
		VkDescriptorSetLayout descSetLayouts[] = { setLayout_ubo };
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = descSetLayouts;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;		//不设置，无法将 layout-scene 复用
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_scene));

		//Solid-Tex
		VkDescriptorSetLayout descSetLayouts_solidTex[] = { setLayout_ubo,setLayout_tex };
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 2;
		pipelineLayoutInfo.pSetLayouts = descSetLayouts_solidTex;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_solidTex));

		//Instancing
		VkDescriptorSetLayout descSetLayouts_Instance[] = { setLayout_ubo,setLayout_ubo,setLayout_tex };
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 3;
		pipelineLayoutInfo.pSetLayouts = descSetLayouts_Instance;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;

		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_instance));

		/* Pipeline <Solid-Tex> <Instancing> */
		std::vector<const char*> shaderFiles = { "shaders/shader.vert.spv", "shaders/shader.frag.spv" };
		createPipeline(device, renderPass, &shaderFiles, &piLayout_solidTex, &pi_SolidTex);
		shaderFiles = { "shaders/instancing.vert.spv", "shaders/instancing.frag.spv" };
		createPipeline(device, renderPass, &shaderFiles, &piLayout_instance, &pi_Instance);
	}
	/* 根据 shader 创建 pipeline */
	void createPipeline(VkDevice device,VkRenderPass renderPass,std::vector<const char*>* shaderFiles,VkPipelineLayout* piLayout,VkPipeline* pi)
	{
		//std::vector<const char*> shaderFiles = { "shaders/shader.vert.spv", "shaders/shader.frag.spv" };
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
		VkPipelineVertexInputStateCreateInfo vertexInputSCI = geos::GeoCube::getVertexInput(&attributes, &bindings);

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
		vkDestroyPipeline(device, pi_SolidTex, nullptr);
		vkDestroyPipeline(device, pi_Instance, nullptr);

		vkDestroyPipelineLayout(device, piLayout_instance, nullptr);
		vkDestroyPipelineLayout(device, piLayout_solidTex, nullptr);
		vkDestroyPipelineLayout(device, piLayout_scene, nullptr);

		vkDestroyDescriptorSetLayout(device, setLayout_ubo, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_tex, nullptr);
	}
};
