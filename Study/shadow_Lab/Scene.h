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
	View* view;
	std::array<InstanceData, countInstance> instances;
	std::array<InstancePose, countInstance> poses;
	std::array<vec4, 6> uiPts;

	std::vector<geos::GeoSquarePillar*> pillars;		//��������ʵ����
	std::vector<geos::GeoCube*> cubes;					//���� 3D-Texture
	geos::Geo* ground;
	geos::GeoCube* cube;
	geos::GeoSphere* sphere;

	float deltaTime;
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
		view = new View({ 0,0,1 }, 9.0f, { -6.0f,6.0f }, extent);
		ground = new geos::GeoPlane(8, { 0,0,-2 });
		ground->prepareBuffer(vulkanDevice);
		sphere = new geos::GeoSphere(0.6f, { 0.2f,0.2f }, 32, 24);
		sphere->prepareBuffer(vulkanDevice);
		/* ������  ����Instancing������ */
		float size = 0.3f;
		vec3 clipA = { 0.5f,0.3f,0.2f };
		vec3 clipB = { 0.4f,0.2f,0.45f };
		cube = new geos::GeoCube(size, clipA, clipB);
		cube->prepareBuffer(vulkanDevice);

		/* ��������ĳ�ʼ λ��-���� */
		ground->pos = { 0,0,0 };
		sphere->pos = { 0,0,0.6f };
		cube->pos = { 0,0,2 };
		ground->modelMatrix = glm::translate(ground->modelMatrix, ground->pos);
		sphere->modelMatrix = glm::translate(sphere->modelMatrix, sphere->pos);
		cube->modelMatrix = glm::translate(cube->modelMatrix, cube->pos);

		/* ���� */
		vec2 baseSize = { 0.3f,0.4f };
		vec2 topSize = { 0.2f,0.25f };
		vec2 heights = { 0.8f,0.8f };   //x �Ǿ���ֵ��y�Ǳ���
		vec2 topCenter = { 0.0f,0.05f };
		vec2 uvPlane = { 1.0f,1.0f };
		Random rnd(0, { 0,1.0f });

		float PI = glm::pi<float>();
		int countPillar =  15;
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
		/* һ�� Cube */
		int countCube = 7;
		float left = 3.2f;
		float space = 0.4f;
		float step = 1.0f / (countCube - 1);
		float linePos = -4.5f;
		for (int i = 0; i < countCube; i++)
		{
			float size = 0.2f + step * i;
			linePos = linePos + size + space;
			auto tmp = new geos::GeoCube(0.2f+step*i, clipA, clipB);
			tmp->prepareBuffer(vulkanDevice);
			tmp->pos = { left,linePos,size/2 };
			tmp->modelMatrix = glm::translate(tmp->modelMatrix, tmp->pos);
			cubes.push_back(tmp);
		}
		/* Instancing�Ļ��� ��ʼ�� */
		glm::mat4 idenity = glm::mat4(1.0f);
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
	}
	/* ����-UBO ��Դ */
	void update(Frame* pFrame, float time, float deltaTime,int opKey)
	{
		this->deltaTime = deltaTime;
		view->update(deltaTime,opKey);
		float ang = time * glm::radians(30.0f);//time�����ʱ��
		updateInstance(ang);
		//��������
		memcpy(pFrame->ubo_ui.mapped, uiPts.data(), sizeof(vec4) * 6);
		memcpy(pFrame->ubo_scene.mapped, &view->data, sizeof(View::UniformBufferObject));
		memcpy(pFrame->ubo_shadow.mapped, &view->shadowData, sizeof(View::ShadowObject));
		memcpy(pFrame->ubo_instance.mapped, instances.data(), sizeof(InstanceData) * countInstance);
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
				/* texture-array */
				for (uint32_t i = 0; i < pillars.size(); i++)
				{
					pod.model = pillars[i]->modelMatrix;
					/* �л���Array�Ĳ�ͬLayer */
					pod.texIndex = { i % 2,0,0,0 };
					vkCmdPushConstants(cmd, piLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, size, &pod);
					pillars[i]->drawGeo(cmd);
				}
			}
			break;
			case 1:
				/* 3d-texture */
				for (uint32_t i = 0; i < cubes.size(); i++)
				{
					//glm::vec4 posVS =glm::vec4( cubes[i]->pos,1.0) * view->data.view;
					pod.model = cubes[i]->modelMatrix;
					pod.texIndex = { i,0,0,0 };
					vkCmdPushConstants(cmd, piLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, size, &pod);
					cubes[i]->drawGeo(cmd);
				}
				break;
			case 2:
				/* instancing */
				vkCmdPushConstants(cmd, piLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, size, &pod);
				cube->drawGeo(cmd,countInstance);
				break;
			case 3:
				/* ���� */
				pod.model = sphere->modelMatrix;
				vkCmdPushConstants(cmd, piLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, size, &pod);
				sphere->drawGeo(cmd);
				break;
			case 4:
				/* texture */
				pod.model = ground->modelMatrix;
				vkCmdPushConstants(cmd, piLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, size, &pod);
				ground->drawGeo(cmd);
				break;
			case 5:
				//�� UI
				vkCmdPushConstants(cmd, piLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, size, &pod);
				vkCmdDraw(cmd, 6, 1, 0, 0);
				break;
		}
	}

	void cleanup()
	{
		ground->clean();
		sphere->clean();
		cube->clean();
		for (uint32_t i = 0; i < pillars.size(); i++)
		{
			pillars[i]->clean();
		}
		for (uint32_t i = 0; i < cubes.size(); i++)
		{
			cubes[i]->clean();
		}
	}
};
