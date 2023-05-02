#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include "descriptors.h"
#include "pipelines.h"
namespace mg 
{
	/*
	��Ӧһ��Pipeline,descriptor-Layout,pipeline-Layout
	���� spv��ʽ��shader,������shader module
	*/
namespace shaders 
{
	VkShaderModule loadShader(const char* fileName, VkDevice device);

	/*
	Shader����
	*/
	struct Shader
	{
		//˳����װ�� descriptor layout ����
		std::vector<VkDescriptorType> typeSet;		// vert:<ubo,tex>	frag:<ubo,tex>
		std::vector<VkShaderStageFlags> stageSet;	// vert				frag
		std::vector<uint32_t> descCountSet;

		/*
		* Using push constants it's possible to pass a small bit of static data to a shader, which is stored in the command buffer stat
		* This is perfect for passing e.g. static per-object data or parameters without the need for descriptor sets
		*/
		//���� pipelineLayout
		std::vector<VkPushConstantRange> pushRanges;
		VkDescriptorSetLayout descLayout;			//vert:<ubo,tex>	frag:<ubo,tex>
		VkPipelineLayout layout;
		//VkPipeline pipeline;

		std::vector<const char*> shaderFiles;			//shader.spv�ļ�
		std::vector<VkShaderStageFlagBits> shaderStageFlags;

		//ʹ�� �ṹ������ ����,���
		void createDescLayout(VkDevice device);
		void createPipelineLayout(VkDevice device);
		//���� shader-layout ��������,�ⲿ����render-pass
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
