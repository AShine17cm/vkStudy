#include "shaders.h"
#include "VulkanTools.h"
#include <iostream>
#include <fstream>
#include <assert.h>

//#include <ios>

namespace mg 
{
namespace shaders 
{
    VkShaderModule loadShader(const char* fileName, VkDevice device)
    {
        std::ifstream is(fileName, std::ios::binary | std::ios::in | std::ios::ate);

        if (is.is_open())
        {
            size_t size = is.tellg();
            is.seekg(0, std::ios::beg);
            char* shaderCode = new char[size];
            is.read(shaderCode, size);
            is.close();

            assert(size > 0);

            VkShaderModule shaderModule;
            VkShaderModuleCreateInfo moduleCreateInfo{};
            moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            moduleCreateInfo.codeSize = size;
            moduleCreateInfo.pCode = (uint32_t*)shaderCode;

            MG_CHECK_RESULT(vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderModule));

            delete[] shaderCode;

            return shaderModule;
        }
        else
        {
            std::cerr << "Error: Could not open shader file \"" << fileName << "\"" << "\n";
            return VK_NULL_HANDLE;
        }
    }

	void Shader::createDescLayout(VkDevice device) 
	{
		mg::descriptors::createDescriptorSetLayout(typeSet.data(), stageSet.data(), descCountSet.data(), typeSet.size(), device,&descLayout,0);
	}
	//管线的 Layout
	void Shader::createPipelineLayout(VkDevice device)
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.pSetLayouts = &descLayout;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pushConstantRangeCount = pushRanges.size();
		pipelineLayoutInfo.pPushConstantRanges = pushRanges.data();

		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &layout));
	}
	void Shader::createPipeline(
		VkPipelineColorBlendAttachmentState* attachmentStates,
		uint32_t attachmentState_Count,
		VkPipelineVertexInputStateCreateInfo* vertexInputSCI,
		VkRenderPass renderPass,
		VkDevice device,
		VkPipeline *pipeline,
		VkPipelineCache pipelineCache,
		void** extends,
		mg::pipelines::MgPipelineEx* extendTypes,
		uint32_t extendCount
	)
	{
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

		createPipelineLayout(device);

		mg::pipelines::createPipeline(
			shaderStages.data(), shaderStages.size(),
			attachmentStates, 1,
			vertexInputSCI,
			layout,
			renderPass,
			device,
			pipelineCache,
			pipeline,
			extends, extendTypes, extendCount);

		//创建pipeline完毕，销毁shader-module
		for (uint32_t i = 0; i < shaderModules.size(); i++)
		{
			vkDestroyShaderModule(device, shaderModules[i], nullptr);
		}
	}

	void Shader::createPipelineDerived(
		VkPipelineColorBlendAttachmentState* attachmentStates,
		uint32_t attachmentState_Count,
		VkPipelineVertexInputStateCreateInfo* vertexInputSCI,
		VkRenderPass renderPass,
		VkDevice device,
		VkPipeline* pipeline,
		VkPipelineCache pipelineCache,
		void** extends,
		mg::pipelines::MgPipelineEx* extendTypes,
		uint32_t extendCount
	)
	{
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

		//createPipelineLayout(device);

		mg::pipelines::createPipeline(
			shaderStages.data(), shaderStages.size(),
			attachmentStates, 1,
			vertexInputSCI,
			layout,
			renderPass,
			device,
			pipelineCache,
			pipeline,
			extends, extendTypes, extendCount);

		//创建pipeline完毕，销毁shader-module
		for (uint32_t i = 0; i < shaderModules.size(); i++)
		{
			vkDestroyShaderModule(device, shaderModules[i], nullptr);
		}
	}
	void Shader::cleanup(VkDevice device)
	{
		//vkDestroyPipeline(device, pipeline, nullptr);
		vkDestroyPipelineLayout(device, layout, nullptr);
		vkDestroyDescriptorSetLayout(device, descLayout, nullptr);
	}
}
}