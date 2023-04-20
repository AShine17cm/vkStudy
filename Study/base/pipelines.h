#pragma once
#include "vulkan/vulkan.h"
#include "VulkanDevice.h"
namespace mg
{
	enum class MgPipelineEx
	{
		None = 0,
		Assembly = 10,
		Rasterization = 20,
		DepthStencil = 30,
		Viewport = 40,
		Viewport_A = 41,
		Multisample = 50,
		DynamicState = 60,
		//DynamicState_A=61,
		//ColorBlend=30,
	};
	namespace pipelines
	{
		//��׼�ӿ�
		void createPipeline(
			VkPipelineShaderStageCreateInfo* shaderStages,
			uint32_t shaderStage_Count,
			VkPipelineColorBlendAttachmentState* attachmentStates,
			uint32_t attachmentState_Count,
			VkPipelineVertexInputStateCreateInfo* vertexInputSCI,
			VkPipelineLayout layout,
			VkRenderPass renderPass,
			VkDevice device,
			VkPipelineCache pipelineCache,
			VkPipeline* pipeline,
			void** extends = nullptr,
			MgPipelineEx* extendTypes = nullptr,
			uint32_t extendCount = 0
		);
		//�������� interleaved
		void createPipeline_Interleaved(
			VkPipelineShaderStageCreateInfo* shaderStages,
			uint32_t shaderStage_Count,
			VkPipelineColorBlendAttachmentState* attachmentStates,
			uint32_t attachmentState_Count,
			VkFormat* vertexFormats,
			uint32_t* vertexAttributeOffsets,
			uint32_t vertexAttributeCount,
			VkPipelineLayout layout,
			VkRenderPass renderPass,
			VkDevice* device,
			VkPipelineCache pipelineCache,
			VkPipeline* pipeline
		);
		//����󶨣�ÿ����һ����������
		void createPipeline_Separated(
			VkPipelineShaderStageCreateInfo* shaderStages,
			uint32_t shaderStage_Count,
			VkPipelineColorBlendAttachmentState* attachmentStates,
			uint32_t attachmentState_Count,
			VkFormat* vertexFormats,
			uint32_t* vertexAttributeSizes,
			uint32_t vertexBindingCount,
			VkPipelineLayout layout,
			VkRenderPass renderPass,
			VkDevice* device,
			VkPipelineCache pipelineCache,
			VkPipeline* pipeline
		);


	}
}
