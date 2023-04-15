#include "OffscreenPass.h"
namespace mg
{
	void OffscreenPass::prepare(VulkanDevice* vulkanDevice,VkFormat format,VkFormat depthFormat,VkRenderPass renderPass)
	{
		//VkFormat format = VK_FORMAT_B8G8R8A8_SRGB;
		//VkFormat depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;

		textures::MgImageInfo info{};
		info.extent3D = { extent.width,extent.height,1 };

		info.formats = {
			VK_IMAGE_TYPE_2D, format,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,//ֱ�Ӵ� color attachment�в���
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
		color = new textures::Texture(vulkanDevice, info);
		info.formats = {
			VK_IMAGE_TYPE_2D,depthFormat,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT| VK_IMAGE_USAGE_SAMPLED_BIT,//��Input_attachment_bit ����ת shader_read_only
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
		depth = new textures::Texture(vulkanDevice, info);
		color->load(nullptr);
		depth->load(nullptr);


		//renderpasses::createRenderPass(format, depthFormat, vulkanDevice->logicalDevice, renderPass);

		VkImageView attachments[] = { color->view,depth->view };
		framebuffers::createFramebuffers(extent, renderPass, attachments, 2, vulkanDevice->logicalDevice, &frameBuffer);
	}
	void OffscreenPass::cleanup(VkDevice device)
	{
		vkDestroyFramebuffer(device, frameBuffer, nullptr);
		color->destroy();
		depth->destroy();
	}
}