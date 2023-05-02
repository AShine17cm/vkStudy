
#include "VulkanDevice.h"
#include "VulkanTools.h"
#include "pipelines.h"

namespace mg
{
	namespace pipelines
	{
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
			void** extends,
			MgPipelineEx* extendTypes,
			uint32_t extendCount
		)
		{
			//三角面
			VkPipelineInputAssemblyStateCreateInfo assemblyStateCI{};
			assemblyStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			assemblyStateCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			assemblyStateCI.primitiveRestartEnable = VK_FALSE;
			assemblyStateCI.flags = 0;
			//栅格化
			VkPipelineRasterizationStateCreateInfo rasterStateCI{};
			rasterStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterStateCI.polygonMode = VK_POLYGON_MODE_FILL;
			rasterStateCI.cullMode = VK_CULL_MODE_BACK_BIT;
			//rasterStateCI.cullMode = VK_CULL_MODE_NONE;
			rasterStateCI.frontFace = VK_FRONT_FACE_CLOCKWISE;
			rasterStateCI.flags = 0;
			rasterStateCI.lineWidth = 1.0f;	//需要默认为 1.0
			//rasterStateCI.depthClampEnable = VK_FALSE;
			//rasterStateCI.rasterizerDiscardEnable = VK_FALSE;
			//rasterStateCI.depthBiasEnable = VK_FALSE;
			//缓冲区的颜色混合
			VkPipelineColorBlendStateCreateInfo colorBlendSCI{};
			colorBlendSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlendSCI.attachmentCount = attachmentState_Count;
			colorBlendSCI.pAttachments = attachmentStates;
			colorBlendSCI.logicOpEnable = VK_FALSE;
			colorBlendSCI.logicOp = VK_LOGIC_OP_COPY;
			colorBlendSCI.blendConstants[0] = 0.0f;
			colorBlendSCI.blendConstants[1] = 0.0f;
			colorBlendSCI.blendConstants[2] = 0.0f;
			colorBlendSCI.blendConstants[3] = 0.0f;
			//深度 模板
			VkPipelineDepthStencilStateCreateInfo depthStencilSCI{};
			depthStencilSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencilSCI.depthTestEnable = VK_TRUE;
			depthStencilSCI.depthWriteEnable = VK_TRUE;
			depthStencilSCI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
			depthStencilSCI.back.compareOp = VK_COMPARE_OP_ALWAYS;

			VkPipelineViewportStateCreateInfo viewportSCI{};
			viewportSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportSCI.scissorCount = 1;
			viewportSCI.viewportCount = 1;
			viewportSCI.flags = 0;
			//viewportSCI.pScissors = nullptr;
			//viewportSCI.pViewports = nullptr;

			VkPipelineMultisampleStateCreateInfo multisampleSCI{};
			multisampleSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampleSCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			multisampleSCI.flags = 0;
			//multisampleSCI.sampleShadingEnable = VK_FALSE;

			//Dynamic State
			std::vector<VkDynamicState> dynamicStates = {
				VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_SCISSOR
			};
			VkPipelineDynamicStateCreateInfo dynamicSCI{};
			dynamicSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicSCI.flags = 0;
			dynamicSCI.dynamicStateCount = dynamicStates.size();
			dynamicSCI.pDynamicStates = dynamicStates.data();

			//图形管线创建
			VkGraphicsPipelineCreateInfo pipelineCI{};
			pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineCI.pDynamicState = &dynamicSCI;
			pipelineCI.pInputAssemblyState = &assemblyStateCI;
			pipelineCI.pMultisampleState = &multisampleSCI;
			pipelineCI.pRasterizationState = &rasterStateCI;
			pipelineCI.pColorBlendState = &colorBlendSCI;
			pipelineCI.pDepthStencilState = &depthStencilSCI;
			pipelineCI.pViewportState = &viewportSCI;
			pipelineCI.pVertexInputState = vertexInputSCI;
			pipelineCI.stageCount = shaderStage_Count;
			pipelineCI.pStages = shaderStages;
			pipelineCI.renderPass = renderPass;
			pipelineCI.subpass = 0;
			pipelineCI.layout = layout;
			pipelineCI.flags = 0;
			pipelineCI.basePipelineIndex = -1;
			pipelineCI.basePipelineHandle = VK_NULL_HANDLE;

			for (uint32_t i = 0; i < extendCount; i++)
			{
				MgPipelineEx type = extendTypes[i];
				switch (type)
				{
				case MgPipelineEx::Assembly:
					pipelineCI.pInputAssemblyState = (VkPipelineInputAssemblyStateCreateInfo*)extends[i];
					break;
				case MgPipelineEx::Rasterization://variablerateshading
					pipelineCI.pRasterizationState = (VkPipelineRasterizationStateCreateInfo*)extends[i];
					break;
				case MgPipelineEx::DepthStencil:
					pipelineCI.pDepthStencilState = (VkPipelineDepthStencilStateCreateInfo*)extends[i];
					break;
				case MgPipelineEx::Viewport:
					pipelineCI.pViewportState = (VkPipelineViewportStateCreateInfo*)extends[i];
					break;
				case MgPipelineEx::Viewport_A://variablerateshading
					viewportSCI.pNext = extends[i];
					break;
				case MgPipelineEx::Multisample:
					pipelineCI.pMultisampleState = (VkPipelineMultisampleStateCreateInfo*)extends[i];
					break;
				case MgPipelineEx::DynamicState:
					pipelineCI.pDynamicState = (VkPipelineDynamicStateCreateInfo*)extends[i];
					break;
				case MgPipelineEx::ShadowMap: //阴影贴图，开启偏移
					rasterStateCI.depthBiasEnable = VK_TRUE;
					dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
					dynamicSCI.dynamicStateCount = dynamicStates.size();
					dynamicSCI.pDynamicStates = dynamicStates.data();
					break;
				default:
					break;
				}
			}

			MG_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCI, nullptr, pipeline));
		}
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
			VkDevice device,
			VkPipelineCache pipelineCache,
			VkPipeline* pipeline
		)
		{
			//创建 顶点绑定数据
			uint32_t idxBinding = 0;
			uint32_t stride = vertexAttributeOffsets[0];

			VkVertexInputBindingDescription binding = { idxBinding,stride,VK_VERTEX_INPUT_RATE_VERTEX };

			std::vector<VkVertexInputAttributeDescription> attributes;
			for (uint32_t i = 0; i < vertexAttributeCount; i++)
			{
				VkVertexInputAttributeDescription attribute =
				{ i,idxBinding,vertexFormats[i],vertexAttributeOffsets[i + 1] };

				attributes.push_back(attribute);
			}

			VkPipelineVertexInputStateCreateInfo vertexInputStateCI{};
			vertexInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputStateCI.vertexBindingDescriptionCount = 1;
			vertexInputStateCI.pVertexBindingDescriptions = &binding;
			vertexInputStateCI.vertexAttributeDescriptionCount = vertexAttributeCount;
			vertexInputStateCI.pVertexAttributeDescriptions = attributes.data();

			//标准接口
			createPipeline(
				shaderStages, shaderStage_Count,
				attachmentStates, attachmentState_Count,
				&vertexInputStateCI,
				layout,
				renderPass,
				device,
				pipelineCache,
				pipeline,
				nullptr, nullptr, 0);
		}

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
			VkDevice device,
			VkPipelineCache pipelineCache,
			VkPipeline* pipeline
		)
		{
			//创建 顶点绑定数据
			std::vector<VkVertexInputBindingDescription> bindings;
			for (uint32_t i = 0; i < vertexBindingCount; i++)
			{
				VkVertexInputBindingDescription binding = { i,vertexAttributeSizes[i],VK_VERTEX_INPUT_RATE_VERTEX };
				bindings.push_back(binding);
			}

			std::vector<VkVertexInputAttributeDescription> attributes;
			for (uint32_t i = 0; i < vertexBindingCount; i++)
			{
				VkVertexInputAttributeDescription attribute = { i,i,vertexFormats[i],0 };
				attributes.push_back(attribute);
			}

			VkPipelineVertexInputStateCreateInfo vertexInputStateCI{};
			vertexInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputStateCI.vertexBindingDescriptionCount = vertexBindingCount;
			vertexInputStateCI.pVertexBindingDescriptions = bindings.data();
			vertexInputStateCI.vertexAttributeDescriptionCount = vertexBindingCount;
			vertexInputStateCI.pVertexAttributeDescriptions = attributes.data();

			//标准接口
			createPipeline(
				shaderStages, shaderStage_Count,
				attachmentStates, attachmentState_Count,
				&vertexInputStateCI,
				layout,
				renderPass,
				device,
				pipelineCache,
				pipeline,
				nullptr, nullptr, 0);
		}

		
	}
}