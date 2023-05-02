#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include "descriptors.h"
#include "pipelines.h"
namespace mg 
{
	/*
	对应一个Pipeline,descriptor-Layout,pipeline-Layout
	加载 spv形式的shader,并返回shader module
	*/
namespace shaders 
{
	VkShaderModule loadShader(const char* fileName, VkDevice device);

	/*
	Shader级别
	*/
	struct Shader
	{
		//顺序组装的 descriptor layout 数据
		std::vector<VkDescriptorType> typeSet;		// vert:<ubo,tex>	frag:<ubo,tex>
		std::vector<VkShaderStageFlags> stageSet;	// vert				frag
		std::vector<uint32_t> descCountSet;

		/*
		* Using push constants it's possible to pass a small bit of static data to a shader, which is stored in the command buffer stat
		* This is perfect for passing e.g. static per-object data or parameters without the need for descriptor sets
		*/
		//构成 pipelineLayout
		std::vector<VkPushConstantRange> pushRanges;
		VkDescriptorSetLayout descLayout;			//vert:<ubo,tex>	frag:<ubo,tex>
		VkPipelineLayout layout;
		//VkPipeline pipeline;

		std::vector<const char*> shaderFiles;			//shader.spv文件
		std::vector<VkShaderStageFlagBits> shaderStageFlags;

		//使用 结构内数据 创建,添加
		void createDescLayout(VkDevice device);
		void createPipelineLayout(VkDevice device);
		//根据 shader-layout 创建管线,外部关联render-pass
		void createPipeline(
			VkPipelineColorBlendAttachmentState* attachmentStates,
			uint32_t attachmentState_Count,
			VkPipelineVertexInputStateCreateInfo* vertexInputSCI,
			VkRenderPass renderPass,
			VkDevice device,
			VkPipeline *pipeline,
			VkPipelineCache pipelineCache = nullptr,
			void** extends = nullptr,
			mg::pipelines::MgPipelineEx* extendTypes = nullptr,
			uint32_t extendCount = 0
		);
		void createPipelineDerived(
			VkPipelineColorBlendAttachmentState* attachmentStates,
			uint32_t attachmentState_Count,
			VkPipelineVertexInputStateCreateInfo* vertexInputSCI,
			VkRenderPass renderPass,
			VkDevice device,
			VkPipeline* pipeline,
			VkPipelineCache pipelineCache = nullptr,
			void** extends = nullptr,
			mg::pipelines::MgPipelineEx* extendTypes = nullptr,
			uint32_t extendCount = 0
		);
		void cleanup(VkDevice device);
	};
}
}
