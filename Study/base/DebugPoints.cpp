#pragma once
#include "DebugPoints.h"
#include "VulkanTools.h"
#include "shaders.h"
#include "pipelines.h"

/* 需要打开 feature::wideLines//xxx
   VkPipelineInputAssemblySCI.topology=VK_PRIMITIVE_TOPOLOGY_POINT_LIST
   VkPipelineRasterizationSCI.polygonMode= VK_POLYGON_MODE_FILL
   gl_PointSize 控制大小
*/
namespace geos 
{
	void DebugPoints::prepare(mg::VulkanDevice* vulkanDevice,VkRenderPass renderPass)
	{
		this->vulkanDevice = vulkanDevice;
		this->device = vulkanDevice->logicalDevice;

		
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		VkPushConstantRange pushRange = { VK_SHADER_STAGE_VERTEX_BIT,0,size };

		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;
		MG_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipeLayout));

		/* 顶点布局 UI的数据 在Descriptor-Set中，不走vertex */
		VkPipelineVertexInputStateCreateInfo vertexInputSCI{};
		vertexInputSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		std::vector<VkShaderStageFlagBits> shaderStageFlags= { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT };
		std::vector<const char*> shaderFiles = { "shaders/point.vert.spv", "shaders/point.frag.spv" };

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
		/* Color Blend*/
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		////////
		//三角面
		VkPipelineInputAssemblyStateCreateInfo assemblyStateCI{};
		assemblyStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		assemblyStateCI.topology =  VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		assemblyStateCI.primitiveRestartEnable = VK_FALSE;
		assemblyStateCI.flags = 0;
		//栅格化
		VkPipelineRasterizationStateCreateInfo rasterStateCI{};
		rasterStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterStateCI.polygonMode = VK_POLYGON_MODE_FILL;//
		rasterStateCI.cullMode = VK_CULL_MODE_NONE;
		rasterStateCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterStateCI.flags = 0;
		rasterStateCI.lineWidth = 1.0;	//需要默认为 1.0

		//缓冲区的颜色混合
		VkPipelineColorBlendStateCreateInfo colorBlendSCI{};
		colorBlendSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendSCI.attachmentCount = 1;
		colorBlendSCI.pAttachments = &colorBlendAttachment;
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
		pipelineCI.pVertexInputState = &vertexInputSCI;
		pipelineCI.stageCount = 2;
		pipelineCI.pStages = shaderStages.data();
		pipelineCI.renderPass = renderPass;
		pipelineCI.subpass = 0;
		pipelineCI.layout = pipeLayout;
		pipelineCI.flags = 0;
		pipelineCI.basePipelineIndex = -1;
		pipelineCI.basePipelineHandle = VK_NULL_HANDLE;

		MG_CHECK_RESULT(vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineCI, nullptr, &pipeline));

		//创建pipeline完毕，销毁shader-module
		for (uint32_t i = 0; i < shaderModules.size(); i++)
		{
			vkDestroyShaderModule(device, shaderModules[i], nullptr);
		}
		shaderModules.clear();
		shaderStages.clear();
	}
	void DebugPoints::draw(VkCommandBuffer cmd,glm::mat4 mvp)
	{
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		for (uint32_t i = 0; i < points.size(); i++)
		{
			Point pt = points[i];
			pt.mvp = mvp;
			vkCmdPushConstants(cmd, pipeLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, size, &pt);
			vkCmdDraw(cmd, 3, 1, 0, 0);
		}
	}
	void DebugPoints::addPoint(Point point) 
	{
		points.push_back(point);
	}
	void DebugPoints::clean()
	{
		vkDestroyPipeline(device, pipeline, nullptr);
		vkDestroyPipelineLayout(device, pipeLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, setLayout, nullptr);
	}
}