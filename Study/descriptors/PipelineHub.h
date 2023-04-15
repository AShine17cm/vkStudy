#pragma once
#include <vector>
#include "vulkan/vulkan.h"
#include "VulkanTools.h"
#include "glm.hpp"
using namespace glm;

struct PipelineHub
{
	VkDescriptorSetLayout setLayout_Scene;
	VkDescriptorSetLayout setLayout_Solid;
	VkDescriptorSetLayout setLayout_Occluder;

	VkPipelineLayout piLayout_scene;
	VkPipelineLayout piLayout_Solid;
	VkPipelineLayout piLayout_Occluder;

	VkPipeline pi_Solid;
	VkPipeline pi_Occluder;

	void prepare(VkDevice device, VkRenderPass renderPass)
	{
		/* Descriptor Layout ��1��Pipeline */
		//��һ����	Set=0,binding=0			��������view-proj		ʹ�ù����� setLayout_scene
		std::vector<VkDescriptorType> types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };
		std::vector<VkShaderStageFlags> stages = { VK_SHADER_STAGE_VERTEX_BIT };
		std::vector<uint32_t> desCounts = { 1};
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_Scene);
		//��������	binding=push_constant	ģ�;���  model
		VkPushConstantRange pushRange = { VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(mat4) };
		/* -----------------------Pipeline-Layout-Scene------------------------------- */
		VkDescriptorSetLayout descSetLayouts[] = { setLayout_Scene };
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = descSetLayouts;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;		//�����ã��޷��� layout-scene ���ã�pushRange �� set 0 ??
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_scene));
		/* -----------------------Pipeline-Layout-Scene------------------------------- */

		//�ڶ�����	Set=1,binding=0			���� data[]-tint-texture
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		stages = { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT };
		desCounts = { 4,1,1 };
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_Solid);
		//��������	binding=push_constant	ģ�;���  model

		/* Pipeline Layout */
		VkDescriptorSetLayout descSetLayouts_A[] = { setLayout_Scene,setLayout_Solid };
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 2;
		pipelineLayoutInfo.pSetLayouts = descSetLayouts_A;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;

		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_Solid));
		/* Descriptor Layout ��2��Pipeline */
		//��һ����	Set=0,binding=0			��������view-proj		ʹ�ù����� setLayout_scene
		//�ڶ�����	Set=1,binding=0			���� tint-texture
		types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		stages = { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT };
		desCounts = { 1,1 };
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_Occluder);
		//��������	binding=push_constant	ģ�;���  model

		/* Pipeline Layout */
		VkDescriptorSetLayout descSetLayouts_B[] = { setLayout_Scene,setLayout_Occluder };
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 2;
		pipelineLayoutInfo.pSetLayouts = descSetLayouts_B;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;

		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_Occluder));
		/* Shader A */
		std::vector<const char*> shaderFiles = { "shaders/vert.spv", "shaders/frag.spv" };
		std::vector<VkShaderStageFlagBits> shaderStageFlags = { VK_SHADER_STAGE_VERTEX_BIT,VK_SHADER_STAGE_FRAGMENT_BIT };
		std::vector<VkShaderModule> shaderModules;
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		//���ļ��� ���� spv��ʽ�� shader
		for (uint32_t i = 0; i < shaderFiles.size(); i++)
		{
			VkShaderModule module = mg::shaders::loadShader(shaderFiles[i], device);
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
		VkPipelineVertexInputStateCreateInfo vertexInputSCI = geos::GeoCube::getVertexInput(&attributes, &bindings);

		/* Color Blend*/
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		mg::pipelines::createPipeline(
			shaderStages.data(), shaderStages.size(),
			&colorBlendAttachment, 1,
			&vertexInputSCI,
			piLayout_Solid,
			renderPass,
			device,
			nullptr,
			&pi_Solid,
			nullptr, nullptr, 0);

		//����pipeline��ϣ�����shader-module
		for (uint32_t i = 0; i < shaderModules.size(); i++)
		{
			vkDestroyShaderModule(device, shaderModules[i], nullptr);
		}

		/* Shader B */
		shaderModules.clear();
		shaderStages.clear();
		shaderFiles = { "shaders/vertB.spv", "shaders/fragB.spv" };
		//���ļ��� ���� spv��ʽ�� shader
		for (uint32_t i = 0; i < shaderFiles.size(); i++)
		{
			VkShaderModule module = mg::shaders::loadShader(shaderFiles[i], device);
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
		attributes.clear();
		bindings.clear();
		vertexInputSCI = geos::GeoCube::getVertexInput(&attributes, &bindings);

		/* Color Blend*/
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		mg::pipelines::createPipeline(
			shaderStages.data(), shaderStages.size(),
			&colorBlendAttachment, 1,
			&vertexInputSCI,
			piLayout_Occluder,
			renderPass,
			device,
			nullptr,
			&pi_Occluder,
			nullptr, nullptr, 0);

		//����pipeline��ϣ�����shader-module
		for (uint32_t i = 0; i < shaderModules.size(); i++)
		{
			vkDestroyShaderModule(device, shaderModules[i], nullptr);
		}
	}

	void cleanup(VkDevice device)
	{
		vkDestroyPipeline(device, pi_Solid, nullptr);
		vkDestroyPipeline(device, pi_Occluder, nullptr);
		vkDestroyPipelineLayout(device, piLayout_Solid, nullptr);
		vkDestroyPipelineLayout(device, piLayout_Occluder, nullptr);
		vkDestroyPipelineLayout(device, piLayout_scene, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_Scene, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_Solid, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_Occluder, nullptr);
	}
};
