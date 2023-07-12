#pragma once
#include "Texture.h"
#include "vulkan/vulkan.h"
#include "VirtualTextureHub.h"

using namespace mg;

struct Resource
{
    textures::Texture* tex_ui;
    textures::Texture* tex_depth;   //场景的深度测试
    textures::Texture* tex_figure;  //模型
    textures::Texture* tex_floor;   //地板
    VirtualTextureHub* vt_TextureHub;

    void prepare(VulkanDevice* vulkanDevice,VkExtent2D swapchainExtent) 
    {
        textures::MgImageInfo imgInfo = { {0,0,1},1,1 };
        /* Depth */
        imgInfo = { {swapchainExtent.width,swapchainExtent.height, 1},1,1 };
        imgInfo.formats = {
            VK_IMAGE_TYPE_2D,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT|VK_IMAGE_USAGE_SAMPLED_BIT,//加Input_attachment_bit 可以转shader_readonly
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
        tex_depth = new textures::Texture(vulkanDevice, imgInfo);
        tex_depth->load(nullptr);

        /* Textures */
        imgInfo.gen_Mips = true;
        imgInfo.formats = {
            VK_IMAGE_TYPE_2D,
            VK_FORMAT_R8G8B8A8_SRGB ,
            VK_IMAGE_USAGE_SAMPLED_BIT ,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
        tex_ui = new textures::Texture(vulkanDevice, imgInfo);
        tex_ui->load("../textures/ui.psd");
        tex_ui->genMips();
        tex_figure = new textures::Texture(vulkanDevice, imgInfo);
        tex_figure->load("../textures/rock 01.jpg");
        tex_figure->genMips();
        tex_floor = new textures::Texture(vulkanDevice, imgInfo);
        tex_floor->load("../textures/ground 01.jpg");
        tex_floor->genMips();

        vt_TextureHub = new VirtualTextureHub(vulkanDevice);
        vt_TextureHub->prepareSparseTexture(4096, 4096, 1, VK_FORMAT_R8G8B8A8_UNORM);
    }
    void update(int numKey) {
        switch (numKey)
        {
        case 1:
            vt_TextureHub->fillRandomPages();
            break;
        case 2:
            vt_TextureHub->flushRandomPages();
            break;
        case 3:
            vt_TextureHub->fillMipTail();
            break;
        }
    }
    void cleanup() 
    {
        tex_ui->destroy();
        tex_figure->destroy();
        tex_floor->destroy();
        tex_depth->destroy();
        vt_TextureHub->clean();
    }
};
