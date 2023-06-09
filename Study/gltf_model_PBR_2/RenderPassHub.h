#pragma once
#include "commands.h"
#include <vector>
#include "vulkan/vulkan.h"
#include "Resource.h"


using namespace glm;
using namespace mg;
/*	RenderPass 
	��RenderPass�󶨵� FrameBuffer-ImageView
	ͬһ�� Depth-Tex ����� FrameBuffer ʹ��
*/
struct RenderPassHub
{
	VulkanDevice* vulkanDevice;
	VkDevice device;

	VkRenderPass renderPass;
	VkRenderPass shadowPass;

	VkFramebuffer shadow_FB;
	std::vector<VkFramebuffer> swapChainFramebuffers;

	mg::SwapChain* swapchain;

	VkExtent2D extent;
	VkExtent2D shadowExtent;

	//MSAA ����������� ͸���ڴ�
	struct MultisampleTarget {
		textures::Texture* color;
		textures::Texture* depth;
	} multisampleTarget;

	void init(VulkanDevice* vulkanDevice,VkSurfaceKHR surface, VkFormat depthFormat) 
	{
		this->vulkanDevice = vulkanDevice;
		this->device = vulkanDevice->logicalDevice;

		extent = { WIDTH,HEIGHT };
		swapchain = new mg::SwapChain(WIDTH, HEIGHT);
		swapchain->createSwapChain(vulkanDevice->physicalDevice, device, surface);

		setupMultisampleTarget(swapchain->imageFormat, depthFormat);				//MSAA ����� render target
		mg::renderpasses::createRenderPass_MSAA(swapchain->imageFormat, depthFormat, device, &renderPass);
		/* ��ȾShadow-Map ���õ� RenderPass */
		mg::renderpasses::createDepthRenderPass(depthFormat, device, &shadowPass);
	}

	void createFrameBuffers(Resource* res) 
	{
		/* Framebuffers */
		swapChainFramebuffers.resize(swapchain->imageCount);
		uint32_t attachmentCount = 4;
		for (uint32_t i = 0; i < swapchain->imageCount; i++)
		{
			VkImageView images[] = 
			{ 
				multisampleTarget.color->view,
				swapchain->imageViews[i],
				multisampleTarget.depth->view,
				res->tex_depth->view
			};//depth ����һ��
			framebuffers::createFramebuffers(swapchain->extent, renderPass, images, attachmentCount,device, &swapChainFramebuffers[i]);
		}
		/* ��Ӱ��ص� FrameBuffer */
		shadowExtent = res->tex_shadow->info.getExtend2D();
		VkImageView shadowViews[] = { res->tex_shadow->view };
		framebuffers::createFramebuffers(shadowExtent, shadowPass, shadowViews, 1,device, &shadow_FB,LIGHT_COUNT);
	}
	//MSAA�� ֡
	void setupMultisampleTarget(VkFormat colorFormat,VkFormat depthFormat)
	{
		textures::MgImageInfo imgInfo = { {WIDTH,HEIGHT,1},1,1 };
		/* Textures */
		imgInfo.layers = 1;
		imgInfo.gen_Mips = false;
		imgInfo.formats = {
			VK_IMAGE_TYPE_2D,
			colorFormat ,
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED };
		imgInfo.formats.sampleCount = VK_SAMPLE_COUNT_4_BIT;
		multisampleTarget.color = new textures::Texture(vulkanDevice, imgInfo);
		multisampleTarget.color->extends.push_back(textures::MgTextureEx::NO_ExtraSetting);
		multisampleTarget.color->load(nullptr);

		imgInfo.formats = {
			VK_IMAGE_TYPE_2D,
			depthFormat ,
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED };
		imgInfo.formats.sampleCount = VK_SAMPLE_COUNT_4_BIT;//formats �Ǹ�С�ṹ�������³�ʼ����
		multisampleTarget.depth = new textures::Texture(vulkanDevice, imgInfo);
		multisampleTarget.depth->extends.push_back(textures::MgTextureEx::NO_ExtraSetting);
		multisampleTarget.depth->load(nullptr);

	}
	void clean() 
	{
		for (auto framebuffer : swapChainFramebuffers)
		{
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}
		vkDestroyFramebuffer(device, shadow_FB, nullptr);

		swapchain->cleanup(device);
		vkDestroyRenderPass(device, renderPass, nullptr);
		vkDestroyRenderPass(device, shadowPass, nullptr);

		multisampleTarget.color->destroy();
		multisampleTarget.depth->destroy();

	}
};
