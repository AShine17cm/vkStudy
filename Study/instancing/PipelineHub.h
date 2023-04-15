#pragma once
#include <vector>
#include "vulkan/vulkan.h"
#include "VulkanTools.h"
#include "glm.hpp"
#include "InstanceByVertexBuffers.h"

using namespace glm;

struct PipelineHub
{
	VkDescriptorSetLayout setLayout_Scene;		//view-proj-light,	push -> model-visible
	VkDescriptorSetLayout setLayout_Solid;		//tint texture
	VkDescriptorSetLayout setLayout_Instance;	//texture 2d array	

	VkPipelineLayout piLayout_scene;
	VkPipelineLayout piLayout_Solid;
	VkPipelineLayout piLayout_Instance;

	VkPipeline pi_Solid;
	VkPipeline pi_Instance;

	void prepare(VkDevice device, VkRenderPass renderPass,uint32_t pushSize)
	{
		/* Descriptor Layout 场景部分 Set=0,binding=0 */
		std::vector<VkDescriptorType> types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };
		std::vector<VkShaderStageFlags> stages = { VK_SHADER_STAGE_VERTEX_BIT };
		std::vector<uint32_t> desCounts = { 1};
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_Scene);
		
		/* Descriptor Layout 模型部分 binding = push_constant */
		VkPushConstantRange pushRange = { VK_SHADER_STAGE_VERTEX_BIT,0,pushSize };
		/* Pipeline Layout: Scene */
		VkDescriptorSetLayout descSetLayouts[] = { setLayout_Scene };
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = descSetLayouts;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;		//不设置，无法将 layout-scene 复用
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_scene));

		/* Descriptor Layout 材质部分 Set=1,binding=0，1 */
		types = {  VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		stages = {  VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT };
		desCounts = { 1,1 };
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_Solid);

		/* Pipeline Layout：Solid */
		VkDescriptorSetLayout descSetLayouts_A[] = { setLayout_Scene,setLayout_Solid };
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 2;
		pipelineLayoutInfo.pSetLayouts = descSetLayouts_A;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;

		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_Solid));

		/* Descriptor Layout 材质部分 Set=1,binding=0 */
		types = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		stages = { VK_SHADER_STAGE_FRAGMENT_BIT };
		desCounts = { 1 };
		mg::descriptors::createDescriptorSetLayout(types.data(), stages.data(), desCounts.data(), types.size(), device, &setLayout_Instance);

		/* Pipeline Layout Occluder */
		VkDescriptorSetLayout descSetLayouts_B[] = { setLayout_Scene,setLayout_Instance };
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 2;
		pipelineLayoutInfo.pSetLayouts = descSetLayouts_B;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;

		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &piLayout_Instance));
		/* Shader Solid */
		std::vector<const char*> shaderFiles = { "shaders/solid.vert.spv", "shaders/solid.frag.spv" };
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
		/* Instancing */
		uint32_t stride = sizeof(float);
		uint32_t instanceStride = sizeof(InstanceData);
		uint32_t binding = 1;
		std::vector<VkVertexInputAttributeDescription> attributes;
		std::vector<VkVertexInputBindingDescription> bindings;
		geos::GeoCube::getVertexInput_Instancing(&attributes, &bindings, instanceStride);
		/*补充一下 Instancing相关的 Attributes */
		attributes.push_back({ 4,binding,VK_FORMAT_R32_SINT,0 });// 每个 Binding 从0 开始计数
		attributes.push_back({ 5,binding,VK_FORMAT_R32G32B32_SFLOAT,stride * 1 });
		//塞个矩阵
		attributes.push_back({ 6,binding,VK_FORMAT_R32G32B32A32_SFLOAT,stride * (1 + 3) });			//mat4-0
		attributes.push_back({ 7,binding,VK_FORMAT_R32G32B32A32_SFLOAT,stride * (1 + 3+4) });		//mat4-1
		attributes.push_back({ 8,binding,VK_FORMAT_R32G32B32A32_SFLOAT,stride * (1 + 3+4+4) });		//mat4-2
		attributes.push_back({ 9,binding,VK_FORMAT_R32G32B32A32_SFLOAT,stride * (1 + 3+4+4+4) });	//mat4-3

		VkPipelineVertexInputStateCreateInfo vertexInputSCI{};
		vertexInputSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputSCI.pVertexAttributeDescriptions = attributes.data();
		vertexInputSCI.pVertexBindingDescriptions = bindings.data();
		/* 只使用一部分 non-instaced相关的binding */
		vertexInputSCI.vertexAttributeDescriptionCount = 4;// attributes.size();
		vertexInputSCI.vertexBindingDescriptionCount = 1;// bindings.size();
		vertexInputSCI.flags = 0;

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

		//创建pipeline完毕，销毁shader-module
		for (uint32_t i = 0; i < shaderModules.size(); i++)
		{
			vkDestroyShaderModule(device, shaderModules[i], nullptr);
		}

		/* Shader Instancing */
		shaderModules.clear();
		shaderStages.clear();
		shaderFiles = { "shaders/instancing.vert.spv", "shaders/instancing.frag.spv" };
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
		vertexInputSCI.vertexAttributeDescriptionCount = attributes.size();
		vertexInputSCI.vertexBindingDescriptionCount = bindings.size();

		mg::pipelines::createPipeline(
			shaderStages.data(), shaderStages.size(),
			&colorBlendAttachment, 1,
			&vertexInputSCI,
			piLayout_Instance,
			renderPass,
			device,
			nullptr,
			&pi_Instance,
			nullptr, nullptr, 0);

		//创建pipeline完毕，销毁shader-module
		for (uint32_t i = 0; i < shaderModules.size(); i++)
		{
			vkDestroyShaderModule(device, shaderModules[i], nullptr);
		}
	}

	void cleanup(VkDevice device)
	{
		vkDestroyPipeline(device, pi_Solid, nullptr);
		vkDestroyPipeline(device, pi_Instance, nullptr);
		vkDestroyPipelineLayout(device, piLayout_Solid, nullptr);
		vkDestroyPipelineLayout(device, piLayout_Instance, nullptr);
		vkDestroyPipelineLayout(device, piLayout_scene, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_Scene, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_Solid, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout_Instance, nullptr);
	}
};
