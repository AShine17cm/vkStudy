#pragma once
#include "Texture.h"
#include "vulkan/vulkan.h"
using namespace mg;

struct Resource
{
    textures::Texture* tex_depth;
    textures::Texture* tex_floor;

    void prepare(VulkanDevice* vulkanDevice,VkExtent2D swapchainExtent) 
    {
        textures::MgImageInfo extent = { {0,0,1},1,1 };

        /* Textures */
        extent.formats = {
            VK_IMAGE_TYPE_2D,
            VK_FORMAT_R8G8B8A8_SRGB ,
            VK_IMAGE_USAGE_SAMPLED_BIT ,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
        tex_floor = new textures::Texture(vulkanDevice, extent);
        tex_floor->load("../textures/checker.jpg");

        extent = { {swapchainExtent.width,swapchainExtent.height, 1},1,1 };
        extent.formats = {
            VK_IMAGE_TYPE_2D,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT|VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL };
        tex_depth = new textures::Texture(vulkanDevice, extent);
        tex_depth->load(nullptr);

    }
    void cleanup() 
    {
        tex_floor->destroy();
        tex_depth->destroy();
    }
};
