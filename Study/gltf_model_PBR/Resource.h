#pragma once
#include "constants.h"
#include "Texture.h"
#include "vulkan/vulkan.h"

using namespace mg;

struct Resource
{
    /* 阴影相关 */

    textures::Texture* tex_shadow;

    textures::Texture* tex_depth;   //场景的深度测试
    textures::Texture* tex_ui;      //一个操作说明
    textures::Texture* tex_array;   //两层贴图 立柱-环阵
    textures::Texture* tex_floor;   //地板
    textures::Texture* tex_cube;    //球体的反射

    void prepare(VulkanDevice* vulkanDevice,VkExtent2D swapchainExtent) 
    {
        textures::MgImageInfo imgInfo = { {0,0,1},1,1 };
        /* 场景深度测试 */
        imgInfo = { {swapchainExtent.width,swapchainExtent.height, 1},1,1 };
        imgInfo.formats = {
            VK_IMAGE_TYPE_2D,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT|VK_IMAGE_USAGE_SAMPLED_BIT,//加Input_attachment_bit 可以转shader_readonly
            //VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
        };
        tex_depth = new textures::Texture(vulkanDevice, imgInfo);
        tex_depth->load(nullptr);
        /* 阴影贴图 */
        imgInfo.extent3D = { SHADOWMAP_DIM,SHADOWMAP_DIM,1 };
        imgInfo.layers = LIGHT_COUNT;
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
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

        tex_ui = new textures::Texture(vulkanDevice, imgInfo);
        tex_ui->load("../textures/ui.psd");
        tex_ui->genMips();
        tex_floor = new textures::Texture(vulkanDevice, imgInfo);
        tex_floor->load("../textures/ground 01.jpg");
        tex_floor->genMips();
        /* 多层贴图 - View要使用 2D-Array格式 */
        imgInfo.layers = 2;
        imgInfo.gen_Mips = true;
        tex_array = new textures::Texture(vulkanDevice, imgInfo);
        tex_array->viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        tex_array->load("../textures/rock 01.jpg");
        tex_array->Insert("../textures/rock 02.jpg", 1, 0);
        tex_array->genMips();
        /* Cube-Map */
        imgInfo.layers = 6;
        imgInfo.gen_Mips = true;
        imgInfo.formats.createFalgs = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        tex_cube = new textures::Texture(vulkanDevice, imgInfo);
        tex_cube->viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        tex_cube->load("../textures/cube 0.jpg");
        tex_cube->Insert("../textures/cube 1.jpg", 1, 0);
        tex_cube->Insert("../textures/cube 2.jpg", 2, 0);
        tex_cube->Insert("../textures/cube 3.jpg", 3, 0);
        tex_cube->Insert("../textures/cube 4.jpg", 4, 0);
        tex_cube->Insert("../textures/cube 5.jpg", 5, 0);
        tex_cube->genMips();
    }
    void cleanup() 
    {
        /* 阴影贴图 */
        tex_shadow->destroy();

        tex_ui->destroy();
        tex_array->destroy();
        tex_floor->destroy();
        tex_depth->destroy();
        tex_cube->destroy();

    }
};
