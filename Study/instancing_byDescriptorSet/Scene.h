#pragma once
#include <vector>
#include "glm.hpp"
#include "Geo.h"
#include "InstanceByDescriptorSet.h"
using namespace mg;

/*
将Geo 以不同的材质 渲染
*/
struct PushData {
    glm::mat4 model;
};
struct  Scene
{
	std::vector<geos::Geo*> sceneObject;
	geos::Geo* ground;
    InstanceByDescriptorSet* instance;
    //创建 组织 geo
	void prepare(VulkanDevice* vulkanDevice,VkDescriptorPool pool,PipelineHub* piHub) 
	{
        /* 立方体  将被Instancing的物体 */
        float size = 0.3f;
        vec3 clipA = { 0.5f,0.3f,0.2f };
        vec3 clipB = { 0.4f,0.2f,0.45f };
        auto cubeA = new geos::GeoCube(size, clipA, clipB);
        cubeA->pos = { -0.5f,0.3f,0.1f };
        auto cubeB = new geos::GeoCube(size, clipA, clipB);
        cubeB->pos = { 0.5f,0.8f,0.15f };

        sceneObject.push_back(cubeA); sceneObject[0]->name = "Small Cube";
        sceneObject.push_back(cubeB); sceneObject[1]->name = "Big Cube";
        for (uint32_t i = 0; i < sceneObject.size(); i++)
        {
            sceneObject[i]->prepareBuffer(vulkanDevice);
        }
        ground = new geos::GeoPlane(2, { 0,0,0 });
        ground->prepareBuffer(vulkanDevice);
        ground->name = "Ground";

        /* instance 展示*/
        instance = new InstanceByDescriptorSet();
        instance->prepare(vulkanDevice,pool,piHub, 8);
	}
    void update(float time) 
    {
        for (uint32_t i = 0; i < sceneObject.size(); i++)
        {
            mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, sceneObject[i]->pos);
            model = glm::rotate(model, glm::radians(60.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            
            sceneObject[i]->modelMatrix = model;
        }
        mat4 model = glm::mat4(1.0f);
        model= glm::translate(model, { 0,0,-0.3f });
        ground->modelMatrix = model;

        instance->update(time);
    }
    /*
    用idx 将 geo 以不同的方式渲染
    */
    void draw(VkCommandBuffer cmd,VkPipelineLayout piLayout,  int batchIdx) 
    {
        uint32_t size = sizeof(PushData);
        PushData data = { glm::mat4(1.0f)};
        uint32_t dstSet = 1;
        switch (batchIdx)
        {
        case 0:
            dstSet = 1;
            vkCmdPushConstants(cmd, piLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, size, &data);//必须 push-constant
            /* 绑定 Instance 数据 */
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piLayout, dstSet, 1, &instance->set_Instances, 0, nullptr);
            instance->draw(cmd);
            break;
        case 1:
            for (uint32_t i = 0; i < sceneObject.size(); i++)
            {
                data.model = sceneObject[i]->modelMatrix;
                vkCmdPushConstants(cmd, piLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, size, &data);
                sceneObject[i]->drawGeo(cmd);
            }
            break;
        case 2:
            data.model = ground->modelMatrix;
            vkCmdPushConstants(cmd, piLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, size, &data);
            ground->drawGeo(cmd);
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
        instance->cleanup();
    }
};
