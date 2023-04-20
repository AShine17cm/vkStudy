#pragma once
#include <vector>
#include "glm.hpp"
#include "Geo.h"
using namespace mg;

/*
将Geo 以不同的材质 渲染
*/
struct  Scene
{
	std::vector<geos::Geo*> sceneObject;
	geos::Geo* ground;
    geos::Geo* backGround;

    //创建 组织 geo
	void prepare(VulkanDevice* vulkanDevice) 
	{
        sceneObject.push_back(new geos::GeoCube(0.2f, { -0.5f,0.3f,0.1f }, { 0,0.2,0 }));
        sceneObject.push_back(new geos::GeoCube(0.3f, { 0.5f,0.8f,0.15f }, { 0,0.2,0 }));
        for (uint32_t i = 0; i < sceneObject.size(); i++)
        {
            sceneObject[i]->prepareBuffer(vulkanDevice);
        }
        ground = new geos::GeoPlane(2, { 0,0,0 });
        ground->prepareBuffer(vulkanDevice);

        backGround = new geos::GeoPlane(2, { -0.0f,-1.5f,0 });
        backGround->prepareBuffer(vulkanDevice);
	}
    void update(float time) 
    {
        for (uint32_t i = 0; i < sceneObject.size(); i++)
        {
            mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, sceneObject[i]->pos);
            model = glm::rotate(model, time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            sceneObject[i]->modelMatrix = model;
        }
        ground->modelMatrix = glm::mat4(1.0f);

        mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, backGround->pos);
        model = glm::rotate(model, glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        backGround->modelMatrix = model;
    }
    /*
    用idx 将 geo 以不同的方式渲染
    */
    void draw(VkCommandBuffer cmd,VkPipelineLayout piLayout,  int batchIdx) 
    {
        //vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, dstSet, 1, descriptorSet, 0, nullptr);
        uint32_t size = sizeof(mat4);
        switch (batchIdx)
        {
        case 0:
            for (uint32_t i = 0; i < sceneObject.size(); i++)
            {
                vkCmdPushConstants(cmd, piLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, size, &sceneObject[i]->modelMatrix);
                sceneObject[i]->drawGeo(cmd);
            }
            break;
        case 1:
            //最后画ground 故意的
            vkCmdPushConstants(cmd, piLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, size, &ground->modelMatrix);
            ground->drawGeo(cmd);
            break;
        case 2:
            vkCmdPushConstants(cmd, piLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, size, &backGround->modelMatrix);
            backGround->drawGeo(cmd);
            break;
        }

    }
    void cleanup()
    {
        for (uint32_t i = 0; i < sceneObject.size(); i++)
        {
            sceneObject[i]->clean();
        }
        ground->clean();
        backGround->clean();
    }
};
