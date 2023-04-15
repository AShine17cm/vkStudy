#pragma once
#include <vector>
#include "glm.hpp"

using namespace mg;

struct  Scene
{
    //���� ��֯ geo
	void prepare(VulkanDevice* vulkanDevice) 
	{

        

	}
    void update(float time) 
    {

    }
    /*
    ��idx �� geo �Բ�ͬ�ķ�ʽ��Ⱦ
    */
    void draw(VkCommandBuffer cmd,VkPipelineLayout piLayout,  int batchIdx) 
    {
        uint32_t size = sizeof(glm::vec4);
        glm::vec4 data = { 1,1,1,1 };

        uint32_t vertexCount = 4;
        uint32_t instanceCount = 1;
        uint32_t firstVertex = 0;
        uint32_t firstInstance = 0;
        switch (batchIdx)
        {
        case 0:
            //https://www.saschawillems.de/blog/2016/08/13/vulkan-tutorial-on-rendering-a-fullscreen-quad-without-buffers/
            vertexCount = 3;
            vkCmdPushConstants(cmd, piLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, size, &data);//���� push-constant
            vkCmdDraw(cmd, vertexCount, instanceCount, firstVertex, firstInstance);
            break;
        case 1:
            vertexCount = 9;
            vkCmdPushConstants(cmd, piLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, size, &data);//���� push-constant
            vkCmdDraw(cmd, vertexCount, instanceCount, firstVertex, firstInstance);
            break;
        }

    }
    void cleanup()
    {

    }
};
