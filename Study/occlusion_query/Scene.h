#pragma once
#include <vector>
#include "glm.hpp"
#include "Geo.h"
using namespace mg;

/*
��Geo �Բ�ͬ�Ĳ��� ��Ⱦ
*/
struct PushData {
    glm::mat4 model;
    float visible;
};
struct  Scene
{
	std::vector<geos::Geo*> sceneObject;
	geos::Geo* ground;
    geos::Geo* occluderPlane;

    //���� ��֯ geo
	void prepare(VulkanDevice* vulkanDevice) 
	{
        vec3 clipA = { 0.5f,0.3f,0.2f };
        vec3 clipB = { 0.4f,0.2f,0.45f };
        sceneObject.push_back(new geos::GeoCube(0.2f, clipA,clipB)); sceneObject[0]->name = "Small Cube";
        sceneObject.push_back(new geos::GeoCube(0.3f, clipA,clipB)); sceneObject[1]->name = "Big Cube";
        for (uint32_t i = 0; i < sceneObject.size(); i++)
        {
            sceneObject[i]->prepareBuffer(vulkanDevice);
        }
        ground = new geos::GeoPlane(2, { 0,0,0 });
        ground->prepareBuffer(vulkanDevice);
        ground->name = "Ground";

        occluderPlane = new geos::GeoPlane(2, { 0,-0.25f,0 });//���� �ڵ�ƽ���λ��
        occluderPlane->prepareBuffer(vulkanDevice);
        occluderPlane->name = "Occluder";
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
        ground->modelMatrix = glm::mat4(1.0f);

        mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, occluderPlane->pos);
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        occluderPlane->modelMatrix = model;
    }
    /*
    ��idx �� geo �Բ�ͬ�ķ�ʽ��Ⱦ
    */
    void draw(VkCommandBuffer cmd,VkPipelineLayout piLayout,  int batchIdx,bool doQuery,VkQueryPool pool) 
    {
        uint32_t size = sizeof(PushData);
        PushData data = { glm::mat4(1.0f),1.0f };
        switch (batchIdx)
        {
        case 0:
            data.model = occluderPlane->modelMatrix;
            vkCmdPushConstants(cmd, piLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, size, &data);
            occluderPlane->drawGeo(cmd);
            break;
        case 1:
            for (uint32_t i = 0; i < sceneObject.size(); i++)
            {
                //if (i == 0)continue;
                uint32_t k = i;//��������
                if (doQuery)vkCmdBeginQuery(cmd, pool, k, VK_FLAGS_NONE);   //��ʼ��ѯ
                data.model = sceneObject[i]->modelMatrix;
                vkCmdPushConstants(cmd, piLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, size, &data);
                sceneObject[i]->drawGeo(cmd);
                if (doQuery)vkCmdEndQuery(cmd, pool, k);                    //������ѯ
            }
            break;
        case 112:
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
        occluderPlane->clean();
    }
};
