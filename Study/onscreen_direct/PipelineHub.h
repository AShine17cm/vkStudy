#pragma once
#include <vector>
#include "vulkan/vulkan.h"
#include "VulkanTools.h"
#include "glm.hpp"

using namespace glm;

struct PipelineHub
{
	VkDescriptorSetLayout setLayout_screen;		
	VkDescriptorSetLayout setLayout_object;
	VkPipelineLayout piLayout_screen;

	VkPipeline pi_screen;
	VkPipeline pi_scrArray;	//将数据写到 Shader中

	void prepare(VkDevice device, VkRenderPass renderPass,uint32_t pushSize)
	{
		std::vector<VkDescriptorType> types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		std::vector<VkShaderStageFlags> stages = { VK_SHADER_STAGE_VERTEX_BIT ,VK_SHADER_STAGE_FRAGMENT_BIT };
		std::vector<uint32_t> desCounts = { 1,1};
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_screen);
		
		VkPushConstantRange pushRange = { VK_SHADER_STAGE_VERTEX_BIT,0,pushSize };
		VkDescriptorSetLayout descSetLayouts[] = { setLayout_screen };
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = descSetLayouts;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;		//不设置，无法将 layout-scene 复用
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_screen));


		/* Shader Solid */
		std::vector<const char*> shaderFiles = { "shaders/screen.vert.spv", "shaders/screen.frag.spv" };
		std::vector<VkShaderStageFlagBits> shaderStageFlags = { VK_SHADER_STAGE_VERTEX_BIT,VK_SHADER_STAGE_FRAGMENT_BIT };
		std::vector<VkShaderModule> shaderModules;
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		//从文件中 加载 spv形式的 shader
		for (uint32_t i = 0; i < shaderFiles.size(); i++)
		{
			VkShaderModule module = mg::shaders::loadShader(shaderFiles[i], device);
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
		//std::vector<VkVertexInputAttributeDescription> attributes;
		//std::vector<VkVertexInputBindingDescription> bindings;
		//geos::GeoCube::getVertexInput(&attributes, &bindings);

		VkPipelineVertexInputStateCreateInfo vertexInputSCI{};
		vertexInputSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputSCI.pVertexAttributeDescriptions = nullptr;
		vertexInputSCI.pVertexBindingDescriptions = nullptr;
		vertexInputSCI.vertexAttributeDescriptionCount = 0;
		vertexInputSCI.vertexBindingDescriptionCount = 0;
		vertexInputSCI.flags = 0;

		/* Color Blend*/
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		mg::pipelines::createPipeline(
			shaderStages.data(), shaderStages.size(),
			&colorBlendAttachment, 1,
			&vertexInputSCI,
			piLayout_screen,
			renderPass,
			device,
			nullptr,
			&pi_screen,
			nullptr, nullptr, 0);

		//创建pipeline完毕，销毁shader-module
		for (uint32_t i = 0; i < shaderModules.size(); i++)
		{
			vkDestroyShaderModule(device, shaderModules[i], nullptr);
		}
		shaderModules.clear();
		shaderStages.clear();

		/* 第二个 Pipeline */
		shaderFiles = { "shaders/array.vert.spv", "shaders/array.frag.spv" };
		//从文件中 加载 spv形式的 shader
		for (uint32_t i = 0; i < shaderFiles.size(); i++)
		{
			VkShaderModule module = mg::shaders::loadShader(shaderFiles[i], device);
			VkPipelineShaderStageCreateInfo shaderStage = {};
			shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStage.stage = shaderStageFlags[i];
			shaderStage.module = module;
			shaderStage.pName = "main";
			assert(shaderStage.module != VK_NULL_HANDLE);

			shaderModules.push_back(module); //存储到 类中
			shaderStages.push_back(shaderStage);
		}
		mg::pipelines::createPipeline(
			shaderStages.data(), shaderStages.size(),
			&colorBlendAttachment, 1,
			&vertexInputSCI,
			piLayout_screen,
			renderPass,
			device,
			nullptr,
			&pi_scrArray,
			nullptr, nullptr, 0);
		for (uint32_t i = 0; i < shaderModules.size(); i++)
		{
			vkDestroyShaderModule(device, shaderModules[i], nullptr);
		}
		shaderModules.clear();
	}

	void cleanup(VkDevice device)
	{
		vkDestroyPipeline(device, pi_scrArray, nullptr);
		vkDestroyPipeline(device, pi_screen, nullptr);
		vkDestroyPipelineLayout(device, piLayout_screen, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_screen, nullptr);
	}
};
