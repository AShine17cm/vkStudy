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
			VkRenderPass* renderPass
		)
		{
			VkAttachmentDescription colorAttachment{};
			colorAttachment.format = format;
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout =  VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;//呈现

			VkAttachmentReference colorAttachmentRef{};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			/* Depth */
			VkAttachmentDescription depthAttachment{};
			depthAttachment.format = depthFormat;
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
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
			renderPassInfo.pAttachments =  attachments.data();
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			MG_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, renderPass));
		}
		//创建 只有一个depth attachment的深度 render-pass
		void createDepthRenderPass(
			VkFormat format,
			VkDevice device,
			VkRenderPass* renderPass
		)
		{
			VkAttachmentDescription attachmentDesc{};
			attachmentDesc.format = format;
			attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
			attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;// Attachment will be transitioned to shader read at render pass end

			VkAttachmentReference attachmentRef{};
			attachmentRef.attachment = 0;
			attachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;// Attachment will be used as depth/stencil during render pass

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
	}
}