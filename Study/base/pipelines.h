#pragma once
#include "vulkan/vulkan.h"
#include "VulkanDevice.h"
namespace mg
{
	namespace pipelines
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

			UI = 101, //不同的Vertex-Layout
			ShadowMap = 201, //需要开启 depth-bias
			Geo_MRT=301,
			Deferred_Compose=401,

			Counter_Clock_Wise=501,
			//DynamicState_A=61,
			//ColorBlend=30,
		};
		//标准接口
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
		//顶点数据 interleaved
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
		//多个绑定，每个绑定一个属性数据
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

