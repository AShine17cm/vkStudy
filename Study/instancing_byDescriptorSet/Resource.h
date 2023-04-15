#pragma once
#include "Texture.h"
#include "vulkan/vulkan.h"
using namespace mg;

struct Resource
{
    textures::Texture* tex_depth;//场景的深度测试
    textures::Texture* tex_figure;
    textures::Texture* tex_floor;
    /* instance 相关 */
    textures::Texture* tex_Instance;

    void prepare(VulkanDevice* vulkanDevice,VkExtent2D swapchainExtent) 
    {
        textures::MgImageInfo extent = { {0,0,1},1,1 };
        /* Depth */
        extent = { {swapchainExtent.width,swapchainExtent.height, 1},1,1 };
        extent.formats = {
            VK_IMAGE_TYPE_2D,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT|VK_IMAGE_USAGE_SAMPLED_BIT,//加Input_attachment_bit 可以转shader_readonly
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
        tex_depth = new textures::Texture(vulkanDevice, extent);
        tex_depth->load(nullptr);

        /* Textures */
        extent.formats = {
            VK_IMAGE_TYPE_2D,
            VK_FORMAT_R8G8B8A8_SRGB ,
            VK_IMAGE_USAGE_SAMPLED_BIT ,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
        tex_figure = new textures::Texture(vulkanDevice, extent);
        tex_figure->load("../textures/figure01.jpg");
        tex_floor = new textures::Texture(vulkanDevice, extent);
        tex_floor->load("../textures/checker.jpg");

        /* 贴图数组 */
        tex_Instance = new textures::Texture(vulkanDevice, extent);
        tex_Instance->load("../textures/checker.jpg");
    }
    void cleanup() 
    {
        tex_figure->destroy();
        tex_floor->destroy();
        tex_depth->destroy();

        tex_Instance->destroy();
    }
};
