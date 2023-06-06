#include "renderpasses.h"
#include "VulkanTools.h"

namespace mg
{
	namespace renderpasses
	{
		//创建 color+depth 的render-pass
		void createRenderPass(
			VkFormat format,
			VkFormat depthFormat,
			VkDevice device,
			VkRenderPass* renderPass,
			VkSampleCountFlagBits SAMPLE_COUNT
		)
		{
			VkAttachmentDescription colorAttachment{};
			colorAttachment.format = format;
			colorAttachment.samples = SAMPLE_COUNT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;//呈现

			VkAttachmentReference colorAttachmentRef{};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			/* Depth */
			VkAttachmentDescription depthAttachment{};
			depthAttachment.format = depthFormat;
			depthAttachment.samples = SAMPLE_COUNT;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			//shader_read_only  需要 usage|INPUT_ATTACHMENT_BIT (sampled_bit 更好用)
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			VkAttachmentReference depthAttachmentRef{};
			depthAttachmentRef.attachment = 1;
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;
			/* Depth */
			subpass.pDepthStencilAttachment = &depthAttachmentRef;


			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			std::array < VkAttachmentDescription, 2 > attachments = { colorAttachment, depthAttachment };
			VkRenderPassCreateInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = attachments.size();
			renderPassInfo.pAttachments = attachments.data();
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			MG_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, renderPass));
		}
		//创建 延迟渲染，后处理 的render-pass
		void create_PostProcess(
			VkFormat format,
			VkDevice device,
			VkRenderPass* renderPass,
			VkSampleCountFlagBits SAMPLE_COUNT
		)
		{
			VkAttachmentDescription colorAttachment{};
			colorAttachment.format = format;
			colorAttachment.samples = SAMPLE_COUNT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;//呈现

			VkAttachmentReference colorAttachmentRef{};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;


			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			std::array < VkAttachmentDescription, 1 > attachments = { colorAttachment };
			VkRenderPassCreateInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = attachments.size();
			renderPassInfo.pAttachments = attachments.data();
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			MG_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, renderPass));
		}
		/* G-Buffer的输出 */
		void create_MRT(
			VkFormat* formats,
			VkDevice device,
			VkRenderPass* renderPass,
			VkSampleCountFlagBits SAMPLE_COUNT
		)
		{
			std::array<VkAttachmentDescription, 4> attachments = {};
			for (int i = 0; i < 4; i++)
			{
				attachments[i].format = formats[i];
				attachments[i].samples = SAMPLE_COUNT;
				attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				if (i == 3)
				{
					attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
					attachments[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;//用 att Ref来修正?
				}
				else
				{
					attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
					attachments[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				}
			}

			std::array<VkAttachmentReference, 3> colorRefs = {};
			colorRefs[0] = { 0,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
			colorRefs[1] = { 1,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
			colorRefs[2] = { 2,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

			VkAttachmentReference depthRef = { 3,VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount =colorRefs.size();
			subpass.pColorAttachments =colorRefs.data();
			subpass.pDepthStencilAttachment = &depthRef;

			std::array<VkSubpassDependency,2> dependencies;
			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;// VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			dependencies[1].srcSubpass = 0;
			dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependencies[1].srcAccessMask= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			//VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT  不支持，用COLOR_ATTACHMENT替代

			VkRenderPassCreateInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = attachments.size();
			renderPassInfo.pAttachments = attachments.data();
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = dependencies.size();
			renderPassInfo.pDependencies = dependencies.data();

			MG_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, renderPass));
		}
		//创建 只有一个depth attachment的深度 render-pass
		void createDepthRenderPass(
			VkFormat format,
			VkDevice device,
			VkRenderPass* renderPass,
			VkSampleCountFlagBits SAMPLE_COUNT
		)
		{
			VkAttachmentDescription attachmentDesc{};
			attachmentDesc.format = format;
			attachmentDesc.samples = SAMPLE_COUNT;
			attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;// Attachment will be transitioned to shader read at render pass end

			VkAttachmentReference attachmentRef{};
			attachmentRef.attachment = 0;
			attachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpassDesc{};
			subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDesc.colorAttachmentCount = 0;
			subpassDesc.pDepthStencilAttachment = &attachmentRef;

			std::vector<VkSubpassDependency> dependecies{ 2 };
			dependecies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependecies[0].dstSubpass = 0;
			dependecies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependecies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependecies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependecies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependecies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			dependecies[1].srcSubpass = 0;
			dependecies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
			dependecies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependecies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependecies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependecies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependecies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			VkRenderPassCreateInfo renderPassCI{};
			renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassCI.attachmentCount = 1;
			renderPassCI.pAttachments = &attachmentDesc;
			renderPassCI.subpassCount = 1;
			renderPassCI.pSubpasses = &subpassDesc;
			renderPassCI.dependencyCount = dependecies.size();
			renderPassCI.pDependencies = dependecies.data();

			MG_CHECK_RESULT(vkCreateRenderPass(device, &renderPassCI, nullptr, renderPass));
		}
		//
				//创建 color+depth 的render-pass
		void createRenderPass_MSAA(
			VkFormat colorFormat,
			VkFormat depthFormat,
			VkDevice device,
			VkRenderPass* renderPass,
			VkSampleCountFlagBits sampleCount
		)
		{
			
			std::array<VkAttachmentDescription, 4> attachments = {};

			// Multisampled attachment that we render to
			attachments[0].format = colorFormat;
			attachments[0].samples = sampleCount;
			attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			// This is the frame buffer attachment to where the multisampled image
			// will be resolved to and which will be presented to the swapchain
			attachments[1].format = colorFormat;
			attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[1].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			// Multisampled depth attachment we render to
			attachments[2].format = depthFormat;
			attachments[2].samples = sampleCount;
			attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[2].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			// Depth resolve attachment
			attachments[3].format = depthFormat;
			attachments[3].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[3].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[3].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkAttachmentReference colorReference = {};
			colorReference.attachment = 0;
			colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentReference depthReference = {};
			depthReference.attachment = 2;
			depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			// Resolve attachment reference for the color attachment
			VkAttachmentReference resolveReference = {};
			resolveReference.attachment = 1;
			resolveReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass = {};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorReference;
			// Pass our resolve attachments to the sub pass
			subpass.pResolveAttachments = &resolveReference;
			subpass.pDepthStencilAttachment = &depthReference;

			std::array<VkSubpassDependency, 2> dependencies;

			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			dependencies[1].srcSubpass = 0;
			dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			VkRenderPassCreateInfo renderPassCI = {};
			renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassCI.attachmentCount = static_cast<uint32_t>(attachments.size());
			renderPassCI.pAttachments = attachments.data();
			renderPassCI.subpassCount = 1;
			renderPassCI.pSubpasses = &subpass;
			renderPassCI.dependencyCount = 2;
			renderPassCI.pDependencies = dependencies.data();
			MG_CHECK_RESULT(vkCreateRenderPass(device, &renderPassCI, nullptr, renderPass));
		}
	}
}