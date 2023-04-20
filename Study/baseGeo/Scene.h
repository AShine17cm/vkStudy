#pragma once
#include <vector>
#include "glm.hpp"
#include "Geo.h"
#include "SimpleRandom.h"
#include "Frame.h"
#include "View.h"

using namespace mg;

/*
��Geo �Բ�ͬ�Ĳ��� ��Ⱦ
*/
struct  Scene
{
	static const int countInstance = 16;
	static const int countTextureArray = 8;
	/* �� PushConstant������ */
	struct PerObjectData
	{
		glm::mat4 model;
		glm::vec4 texIndex;
	};
	/* ��Ⱦ���� */
	struct InstanceData
	{
		glm::mat4 model;
		glm::vec4 texIndex;	//alignas int ��shader��ȡ������ȷ��ֵ?
	};
	/* �����˶� */
	struct  InstancePose
	{
		glm::vec3 pos;		//δʹ��
		glm::vec3 rot;		//δʹ��
		float spd;
	};
	std::array<InstanceData, countInstance> instances;
	std::array<InstancePose, countInstance> poses;
	std::array<vec4, 6> uiPts;

	View* view;

	geos::Geo* ground;
	geos::GeoCube* cube;

	std::vector<geos::GeoSquarePillar*> pillars;		//��������ʵ����

	//���� ģ��
	void prepare(VulkanDevice* vulkanDevice, VkExtent2D extent)
	{
		/* ui�� */
		uiPts[0] = { -0.95,-0.7f,0,1 };
		uiPts[1] = { -0.95,-0.95,0,0 };
		uiPts[2] = { -0.45f,-0.95,1,0 };

		uiPts[3] = { -0.45f,-0.7f,1,1 };
		uiPts[4] = uiPts[0];
		uiPts[5] = uiPts[2];
		/* ������� */
		view = new View({ 0,0,1 }, 9.0f, { -3.0f,6.0f }, extent);
		ground = new geos::GeoPlane(8, { 0,0,-2 });
		ground->prepareBuffer(vulkanDevice);

		/* ���� */
		vec2 baseSize = { 0.3f,0.4f };
		vec2 topSize = { 0.2f,0.25f };
		vec2 heights = { 0.8f,0.8f };   //x �Ǿ���ֵ��y�Ǳ���
		vec2 topCenter = { 0.0f,0.05f };
		vec2 uvPlane = { 1.0f,1.0f };

		Random rnd(0, { 0,1.0f });

		float PI = glm::pi<float>();
		int countPillar = 15;
		float radius = 2.0f;
		float stepAng = PI * 2.0 / countPillar;
		float ang;
		/* �������� */
		for (int i = 0; i < countPillar; i++)
		{
			ang = stepAng * i;
			/* ��һЩ��� ��״-�뾶-�Ƕ� */
			vec2 base = baseSize + glm::vec2{ rnd.generate() * 0.2f, rnd.generate() * 0.2f };
			vec2 top = topSize + glm::vec2{ rnd.generate() * 0.2f, rnd.generate() * 0.2f };
			vec2 height = heights + glm::vec2{ rnd.generate() * 0.2f, rnd.generate() * 0.2f };
			float r = radius + rnd.generate() * 0.5f;
			ang = ang * (1.0f + (rnd.generate() - 0.5f) * 0.04f);
			float sin = r * glm::sin(ang);
			float cos = r * glm::cos(ang);

			auto  pillar = new geos::GeoSquarePillar(base, top, topCenter, height, uvPlane);
			pillar->prepareBuffer(vulkanDevice);
			pillar->pos = { cos,sin,0 };
			pillar->modelMatrix = glm::translate(pillar->modelMatrix, pillar->pos);
			pillar->modelMatrix = glm::rotate(pillar->modelMatrix, ang, { 0,0,1.0f });
			pillars.push_back(pillar);
		}

		glm::mat4 idenity = glm::mat4(1.0f);
		/* ������  ����Instancing������ */
		float size = 0.3f;
		vec3 clipA = { 0.5f,0.3f,0.2f };
		vec3 clipB = { 0.4f,0.2f,0.45f };
		cube = new geos::GeoCube(size, clipA, clipB);
		cube->prepareBuffer(vulkanDevice);
		/* Instancing�Ļ��� ��ʼ�� */
		radius = 1.6f;
		stepAng = PI*2.0f / countInstance;
		float instanceH = 0.6f;
		for (int i = 0; i < countInstance; i++) 
		{
			float r = radius;// +rnd.generate() * 0.5f;
			ang = stepAng * i;
			float sin = r * glm::sin(ang);
			float cos = r * glm::cos(ang);

			glm::mat4 model=glm::translate(idenity, { cos,sin,instanceH });
			ang = ang * (1.0f + (rnd.generate() - 0.5f) * 0.2f);
			model= glm::rotate(model, ang, { 0,0,1.0f });

			InstanceData data{ model,{i % countTextureArray,0,0,0} };
			instances[i] = data;
			float spd = rnd.generate();
			poses[i].spd=spd;
		}

		/* ��������ĳ�ʼ λ��-���� */
		ground->pos = { 0,0,0 };
		cube->pos = {0,0,2};

		ground->modelMatrix = glm::translate(ground->modelMatrix, ground->pos);
		cube->modelMatrix = glm::translate(cube->modelMatrix, cube->pos);

		/* ��ˮƽ���� ��תģ�� */
		geos::Geo* target = nullptr;
		if (nullptr != target)
		{
			//target->modelMatrix= glm::rotate(target->modelMatrix, glm::radians(68.0f), glm::vec3(0.0f, 0, 1.0f));
		}
	}
	/* ����-UBO ��Դ */
	void update(Frame* pFrame, float time, float deltaTime,int opKey)
	{
		view->update(deltaTime,opKey);
		float ang = time * glm::radians(30.0f);//time�����ʱ��

		updateInstance(ang);
		// 
		//��������
		memcpy(pFrame->ui_ubo.mapped, uiPts.data(), sizeof(vec4) * 6);
		memcpy(pFrame->view_ubo.mapped, &view->data, sizeof(View::UniformBufferObject));
		memcpy(pFrame->instance_ubo.mapped, instances.data(), sizeof(InstanceData) * countInstance);
	}
	void updateInstance(float offsetAng) 
	{
		float PI = glm::pi<float>();
		float stepAng = PI * 2.0 / countInstance;
		float radius = 1.5f;
		float ang;
		float instanceH = 0.6f;
		glm::mat4 idenity = glm::mat4(1.0f);
		for (int i = 0; i < countInstance; i++)
		{
			ang =offsetAng+ stepAng * i;

			float sin = radius * glm::sin(ang);
			float cos = radius * glm::cos(ang);
			glm::mat4 model = instances[i].model;
			glm::mat4 rt = glm::rotate(idenity, poses[i].spd, { 1,0,0 });
			//vec3 pos = { model[3].x,model[3].y,model[3].z };
			vec3 pos = { cos,sin,instanceH };
			model = glm::translate(idenity,pos);
			model = glm::rotate(model, ang, { 0,0,1 });
			instances[i].model = model*rt;

		}
	}
	/*
	��idx �� geo �Բ�ͬ�ķ�ʽ��Ⱦ
	*/
	void draw(VkCommandBuffer cmd, VkPipelineLayout piLayout, int batchIdx)
	{
		PerObjectData pod = { glm::mat4(1),{0,0,0,0} };
		uint32_t size = sizeof(PerObjectData);
		VkDeviceSize offsets[] = { 0 };
		switch (batchIdx)
		{
			case 0:
			{
				for (uint32_t i = 0; i < pillars.size(); i++)
				{
					pod.model = pillars[i]->modelMatrix;
					vkCmdPushConstants(cmd, piLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, size, &pod);
					pillars[i]->drawGeo(cmd);
				}
			}
			break;
			case 1:
				/* ʵ������Ⱦ */
				vkCmdPushConstants(cmd, piLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, size, &pod);
				cube->drawGeo(cmd,countInstance);
				break;
			case 2:
				//���ground �����
				pod.model = ground->modelMatrix;
				vkCmdPushConstants(cmd, piLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, size, &pod);
				ground->drawGeo(cmd);
				break;
			case 3:
				//�� UI
				vkCmdPushConstants(cmd, piLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, size, &pod);
				vkCmdDraw(cmd, 6, 1, 0, 0);
				break;
		}

	}
	void cleanup()
	{
		ground->clean();
		for (uint32_t i = 0; i < pillars.size(); i++)
		{
			pillars[i]->clean();
		}
		cube->clean();
	}
};
