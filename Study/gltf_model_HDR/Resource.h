#pragma once
#include "constants.h"
#include "Texture.h"
#include "vulkan/vulkan.h"

using namespace mg;

struct Resource
{
    /* ��Ӱ��� */

    textures::Texture* tex_shadow;

    textures::Texture* tex_depth;   //��������Ȳ���
    textures::Texture* tex_sand;   


    void prepare(VulkanDevice* vulkanDevice,VkExtent2D swapchainExtent) 
    {
        textures::MgImageInfo imgInfo = { {0,0,1},1,1 };
        /* ������Ȳ��� */
        imgInfo = { {swapchainExtent.width,swapchainExtent.height, 1},1,1 };
        imgInfo.formats = {
            VK_IMAGE_TYPE_2D,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT|VK_IMAGE_USAGE_SAMPLED_BIT,//��Input_attachment_bit ����תshader_readonly
            //VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
        };
        tex_depth = new textures::Texture(vulkanDevice, imgInfo);
        tex_depth->load(nullptr);
        /* ��Ӱ��ͼ */
        imgInfo.extent3D = { SHADOWMAP_DIM,SHADOWMAP_DIM,1 };
        imgInfo.layers = LIGHT_COUNT;
        imgInfo.formats.samplerMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        imgInfo.formats.sampleCount = VK_SAMPLE_COUNT_1_BIT;//��ԭ����Ĳ���
        tex_shadow = new textures::Texture(vulkanDevice, imgInfo);
        tex_shadow->viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        tex_shadow->extends = { textures::MgTextureEx::Sampler_ShadowMap };
        tex_shadow->load(nullptr);

        /* Textures */
        imgInfo.layers = 1;
        imgInfo.gen_Mips = true;
        imgInfo.formats = {
            VK_IMAGE_TYPE_2D,
            VK_FORMAT_R8G8B8A8_SRGB ,
            VK_IMAGE_USAGE_SAMPLED_BIT ,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
        imgInfo.formats.samplerMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        tex_sand = new textures::Texture(vulkanDevice, imgInfo);
        tex_sand->load("../textures/sand.psd");
        tex_sand->genMips();
    }
    void cleanup() 
    {
        /* ��Ӱ��ͼ */
        tex_shadow->destroy();
        tex_sand->destroy();
        tex_depth->destroy();
    }
};