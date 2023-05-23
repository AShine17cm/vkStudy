#include "SwapChain.h"
#include "VulkanTools.h"
#include "textures.h"
#include "framebuffers.h"

#include <vector>

namespace mg 
{
    SwapChain::SwapChain(uint32_t width, uint32_t height) 
    {
        this->extent = { width,height };
    }
	void SwapChain::createSwapChain(VkPhysicalDevice physicalDevice,VkDevice device, VkSurfaceKHR surface)
	{
        //将窗口image 2D大小写入 details
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);
        //选择一个 surface-format 和 present-mode
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
        std::vector< VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());

        VkSurfaceFormatKHR surfaceFormat = formats[0];
        for (const auto& sFormat : formats)
        {
            if (sFormat.format == VK_FORMAT_B8G8R8A8_SRGB && sFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                surfaceFormat = sFormat;
            }
        }
        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
        for (const auto& mode : presentModes)
        {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                presentMode = mode;
            }
        }

        imageCount = capabilities.minImageCount +1;//3

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;// | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.preTransform = capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;
        
        MG_CHECK_RESULT(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain));

        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        images.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, images.data());

        imageFormat = surfaceFormat.format;
        textures::MgImgViewInfo viewInfo{};
        viewInfo.imgFormat = imageFormat;
        //swapchain的image views
        imageViews.resize(imageCount);
        for (size_t i = 0; i < imageCount; i++)
        {
            textures::createImageView(device, images[i], &imageViews[i], viewInfo);
        }
	}

    void SwapChain::cleanup(VkDevice device)
    {
        for (auto imageView : imageViews) 
        {
            vkDestroyImageView(device, imageView, nullptr);
        }
        vkDestroySwapchainKHR(device, swapChain, nullptr);
    }
}