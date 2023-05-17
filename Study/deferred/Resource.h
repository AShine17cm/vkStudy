#pragma once
#include "constants.h"
#include "Texture.h"
#include "vulkan/vulkan.h"
#include "SubImageView.h"
#include "tex3D.h"

using namespace mg;
/* 大型资源 */
struct Resource
{
    /* 阴影相关 */
    textures::Texture* tex_shadow;

    /* Geometry-Buffer */
    textures::Texture* geo_normal;
    textures::Texture* geo_pos;
    textures::Texture* geo_albedo;
    textures::Texture* geo_depth;   //场景的深度测试

    textures::Texture* tex_ui;      //一个操作说明
    textures::Texture* tex_array;   //两层贴图 立柱-环阵
    textures::Texture* tex_mips;    //mip-maps 立方体-旋转环阵
    textures::Texture* tex_floor;   //地板
    textures::Texture* tex_cube;    //球体的反射
    /* 法线 */
    textures::Texture* norm_01;
    textures::Texture* norm_02;

    SubImageView* subView;          //Image的部分 <layer,mip-level>, 键盘<1,2>切换<tex_mips,subView>

    void prepare(VulkanDevice* vulkanDevice,VkExtent2D swapchainExtent) 
    {
        textures::MgImageInfo imgInfo = { {0,0,1},1,1 };
        /*  */
        imgInfo = { {swapchainExtent.width,swapchainExtent.height, 1},1,1 };
        imgInfo.formats = {
            VK_IMAGE_TYPE_2D,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT|VK_IMAGE_USAGE_SAMPLED_BIT,//加Input_attachment_bit 可以转shader_readonly
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
        };
        /* 阴影贴图 */
        imgInfo.extent3D = { SHADOWMAP_DIM_W,SHADOWMAP_DIM_W,1 };
        imgInfo.layers = LIGHT_COUNT;
        tex_shadow = new textures::Texture(vulkanDevice, imgInfo);
        tex_shadow->viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        tex_shadow->extends = { textures::MgTextureEx::Sampler_ShadowMap };
        tex_shadow->load(nullptr);

        imgInfo.extent3D = { GEOMETRY_DIM_W,GEOMETRY_DIM_H,1 };
        
        imgInfo.layers = 1;
        /* 场景深度测试 */
        geo_depth = new textures::Texture(vulkanDevice, imgInfo);
        geo_depth->load(nullptr);
        /* G-Buffer */
        imgInfo.formats = {
            VK_IMAGE_TYPE_2D,
            VK_FORMAT_R16G16B16A16_SFLOAT ,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_SAMPLED_BIT ,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        geo_normal = new textures::Texture(vulkanDevice, imgInfo);
        geo_normal->load(nullptr);
        geo_pos = new textures::Texture(vulkanDevice, imgInfo);
        geo_pos->load(nullptr);
        imgInfo.formats.format = VK_FORMAT_R8G8B8A8_UNORM;
        geo_albedo = new textures::Texture(vulkanDevice, imgInfo);
        geo_albedo->load(nullptr);
        /* 延迟渲染  Compose阶段 不能使用Image创建时的Layout */
        geo_pos->overrideDescriptor(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        geo_normal->overrideDescriptor(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        geo_albedo->overrideDescriptor(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

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

        /* 带mip-maps的贴图 */
        imgInfo.mipLevels = 8;
        imgInfo.gen_Mips = false;
        tex_mips = new textures::Texture(vulkanDevice, imgInfo);
        tex_mips->load("../textures/rock 03.jpg");
        tex_mips->Insert("../textures/mip 256.jpg", 0, 1);
        tex_mips->Insert("../textures/mip 128.jpg", 0, 2);
        tex_mips->Insert("../textures/mip 64.jpg", 0, 3);
        tex_mips->Insert("../textures/mip 32.jpg", 0, 4);
        tex_mips->Insert("../textures/mip 16.jpg", 0, 5);
        tex_mips->Insert("../textures/mip 8.jpg", 0, 6);
        tex_mips->Insert("../textures/mip 4.jpg", 0, 7);
        /* 多层贴图 - View要使用 2D-Array格式 */
        imgInfo.layers = 2;
        imgInfo.gen_Mips = true;
        tex_array = new textures::Texture(vulkanDevice, imgInfo);
        tex_array->viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        tex_array->load("../textures/rock 01.jpg");
        tex_array->Insert("../textures/rock 02.jpg", 1, 0);
        tex_array->genMips();
        /* 只包含部分 <layer,mip-level>的视图 */
        textures::MgImgViewInfo viewInfo{};
        viewInfo.imgFormat = tex_mips->info.formats.format;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.baseMipLevel = 2;
        viewInfo.mipLevCount = 3;
        subView = new SubImageView(tex_mips, viewInfo);
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
        /* 法线贴图 */
        imgInfo.layers = 1;
        imgInfo.gen_Mips = true;
        imgInfo.formats = {
            VK_IMAGE_TYPE_2D,
            VK_FORMAT_R8G8B8A8_UNORM,// ???
            VK_IMAGE_USAGE_SAMPLED_BIT ,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
        imgInfo.formats.createFalgs = 0;
        norm_01 = new textures::Texture(vulkanDevice, imgInfo);
        norm_01->load("../textures/normal_01.png");
        norm_01->genMips();
        norm_02 = new textures::Texture(vulkanDevice, imgInfo);
        norm_02->load("../textures/normal_02.png");
        norm_02->genMips();
    }
    void cleanup() 
    {
        /* 阴影贴图 */
        tex_shadow->destroy();

        subView->clean(tex_mips->vulkanDevice->logicalDevice);
        tex_ui->destroy();
        tex_array->destroy();
        tex_mips->destroy();
        tex_floor->destroy();
        tex_cube->destroy();

        geo_normal->destroy();
        geo_pos->destroy();
        geo_albedo->destroy();
        geo_depth->destroy();
        /* 法线贴图 */
        norm_01->destroy();
        norm_02->destroy();
    }
};
