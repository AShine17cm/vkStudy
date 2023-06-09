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
	VkRenderPass uiPass;//����
	VkRenderPass shadowPass;

	VkFramebuffer shadow_FB;
	std::vector<VkFramebuffer> swapChainFramebuffers;

	mg::SwapChain* swapchain;

	VkExtent2D extent;
	VkExtent2D shadowExtent;

	VkFormat hdrFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
	//������Ⱦ MSAA ����������� ͸���ڴ�
	struct MultisampleTarget 
	{
		textures::Texture* color;
		textures::Texture* color_resolved;
		textures::Texture* depth;
		textures::Texture* depth_resolved;
		VkFramebuffer frameBuffer;
		VkRenderPass renderPass;
	} msaaTarget;

	//�ع���� ����߹ⲿ��
	struct
	{
		textures::Texture* color0;
		textures::Texture* color1;
		VkFramebuffer frameBuffer;
		VkRenderPass renderPass;
		VkSampler sampler;
	}offscreen;

	struct 
	{
		VkFramebuffer frameBuffer;
		textures::Texture* color;
		VkRenderPass renderPass;
		VkSampler sampler;
	}bloomPass;


	void init(VulkanDevice* vulkanDevice,VkSurfaceKHR surface, VkFormat depthFormat) 
	{
		this->vulkanDevice = vulkanDevice;
		this->device = vulkanDevice->logicalDevice;

		extent = { WIDTH,HEIGHT };
		swapchain = new mg::SwapChain(WIDTH, HEIGHT);
		swapchain->createSwapChain(vulkanDevice->physicalDevice, device, surface);

		//MSAA
		createMsaa_Targets(hdrFormat, depthFormat);
		mg::renderpasses::createRenderPass_MSAA(hdrFormat, depthFormat, device, &msaaTarget.renderPass,VK_SAMPLE_COUNT_4_BIT, VK_FALSE);
		//�߶�̬ ӳ�� �Ͷ�̬
		createOffscreen_Targets();
		mg::renderpasses::createRenderPass_HDR(VK_FORMAT_R32G32B32A32_SFLOAT, depthFormat, device, &offscreen.renderPass);
		mg::renderpasses::createRenderPass_Bloom(VK_FORMAT_R32G32B32A32_SFLOAT, device, &bloomPass.renderPass);
		//Ldr ����
		mg::renderpasses::create_PostProcess(swapchain->imageFormat, device, &renderPass);

		/* ��ȾShadow-Map ���õ� RenderPass */
		mg::renderpasses::createDepthRenderPass(depthFormat, device, &shadowPass);
		uiPass = renderPass;
	}
	//MSAA�� ֡
	void createMsaa_Targets(VkFormat colorFormat, VkFormat depthFormat)
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
		msaaTarget.color = new textures::Texture(vulkanDevice, imgInfo);
		msaaTarget.color->extends.push_back(textures::MgTextureEx::NO_ExtraSetting);
		msaaTarget.color->load(nullptr);

		imgInfo.formats.usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;//�ɲ���
		imgInfo.formats.sampleCount = VK_SAMPLE_COUNT_1_BIT;
		imgInfo.formats.imagelayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		msaaTarget.color_resolved = new textures::Texture(vulkanDevice, imgInfo);
		msaaTarget.color_resolved->extends.push_back(textures::MgTextureEx::NO_ExtraSetting);
		msaaTarget.color_resolved->load(nullptr);
		//Depth
		imgInfo.formats = {
			VK_IMAGE_TYPE_2D,
			depthFormat ,
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED };
		imgInfo.formats.sampleCount = VK_SAMPLE_COUNT_4_BIT;//formats �Ǹ�С�ṹ�������³�ʼ����
		msaaTarget.depth = new textures::Texture(vulkanDevice, imgInfo);
		msaaTarget.depth->extends.push_back(textures::MgTextureEx::NO_ExtraSetting);
		msaaTarget.depth->load(nullptr);

		imgInfo.formats.usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imgInfo.formats.sampleCount = VK_SAMPLE_COUNT_1_BIT;//formats �Ǹ�С�ṹ�������³�ʼ����
		msaaTarget.depth_resolved = new textures::Texture(vulkanDevice, imgInfo);
		msaaTarget.depth_resolved->extends.push_back(textures::MgTextureEx::NO_ExtraSetting);
		msaaTarget.depth_resolved->load(nullptr);
	}
	void createFrameBuffers(Resource* res) 
	{
		/* Framebuffers */
		//MSAA
		VkImageView images[] =
		{
			msaaTarget.color->view,
			msaaTarget.color_resolved->view,
			msaaTarget.depth->view,
			msaaTarget.depth_resolved->view
		};
		framebuffers::createFramebuffers(swapchain->extent, msaaTarget.renderPass, images, 4, device, &msaaTarget.frameBuffer);
		//Offscreen
		VkImageView offscreenImgs[] =
		{
			offscreen.color0->view,
			offscreen.color1->view,
		};
		framebuffers::createFramebuffers(swapchain->extent, offscreen.renderPass, offscreenImgs, 2, device, &offscreen.frameBuffer);
		textures::createSampler(vulkanDevice, 1, &offscreen.sampler, nullptr, 0, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
		//Bloom
		VkImageView bloomImgs[] =
		{
			bloomPass.color->view
		};
		framebuffers::createFramebuffers(swapchain->extent, bloomPass.renderPass, bloomImgs, 1, device, &bloomPass.frameBuffer);
		textures::createSampler(vulkanDevice, 1, &bloomPass.sampler, nullptr, 0, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

		//Ldr
		swapChainFramebuffers.resize(swapchain->imageCount);
		for (uint32_t i = 0; i < swapchain->imageCount; i++)
		{
			VkImageView images[] =
			{
				swapchain->imageViews[i],
			};//depth ����һ��
			framebuffers::createFramebuffers(swapchain->extent, renderPass, images, 1, device, &swapChainFramebuffers[i]);
		}

		/* ��Ӱ��ص� FrameBuffer */
		shadowExtent = res->tex_shadow->info.getExtend2D();
		VkImageView shadowViews[] = { res->tex_shadow->view };
		framebuffers::createFramebuffers(shadowExtent, shadowPass, shadowViews, 1,device, &shadow_FB,LIGHT_COUNT);
	}
	//�߶�̬ ӳ�� �Ͷ�̬���������룬Bloom֡
	void createOffscreen_Targets( )
	{
		textures::MgImageInfo imgInfo = { {WIDTH,HEIGHT,1},1,1 };
		/* Textures */
		imgInfo.layers = 1;
		imgInfo.gen_Mips = false;
		imgInfo.formats = {
			VK_IMAGE_TYPE_2D,
			VK_FORMAT_R32G32B32A32_SFLOAT ,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT| VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		//imgInfo.formats.usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;//�ɲ���
		//imgInfo.formats.sampleCount = VK_SAMPLE_COUNT_1_BIT;
		//imgInfo.formats.imagelayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		offscreen.color0 = new textures::Texture(vulkanDevice, imgInfo);
		offscreen.color0->extends.push_back(textures::MgTextureEx::NO_ExtraSetting);
		offscreen.color0->load(nullptr);

		offscreen.color1 = new textures::Texture(vulkanDevice, imgInfo);
		offscreen.color1->extends.push_back(textures::MgTextureEx::NO_ExtraSetting);
		offscreen.color1->load(nullptr);

		//Bloom
		bloomPass.color = new textures::Texture(vulkanDevice, imgInfo);
		bloomPass.color->extends.push_back(textures::MgTextureEx::NO_ExtraSetting);
		bloomPass.color->load(nullptr);
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

		//MSAA
		msaaTarget.color->destroy();
		msaaTarget.color_resolved->destroy();
		msaaTarget.depth->destroy();
		msaaTarget.depth_resolved->destroy();
		vkDestroyRenderPass(device,msaaTarget.renderPass,nullptr);
		vkDestroyFramebuffer(device, msaaTarget.frameBuffer, nullptr);

		offscreen.color0->destroy();
		offscreen.color1->destroy();

		vkDestroyFramebuffer(device, offscreen.frameBuffer, nullptr);
		vkDestroyRenderPass(device, offscreen.renderPass, nullptr);
		vkDestroySampler(device, offscreen.sampler, nullptr);

		bloomPass.color->destroy();
		vkDestroyFramebuffer(device, bloomPass.frameBuffer, nullptr);
		vkDestroyRenderPass(device, bloomPass.renderPass, nullptr);
		vkDestroySampler(device, bloomPass.sampler, nullptr);
	}
};
