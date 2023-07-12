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
	static const int countTextureArray = 8;
	/* �� PushConstant������ */
	struct PerObjectData
	{
		glm::mat4 model;
		glm::vec4 texIndex;
	};
	std::array<vec4, 6> uiPts;

	View* view;

	geos::Geo* ground;
	geos::GeoSphere* sphere;

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
		sphere = new geos::GeoSphere(0.6f, { 0.2f,0.2f }, 32, 24);
		sphere->prepareBuffer(vulkanDevice);

		Random rnd(0, { 0,1.0f });

		/* ��������ĳ�ʼ λ��-���� */
		ground->pos = { 0,0,0 };
		sphere->pos = { 0,0,1 };
		ground->modelMatrix = glm::translate(ground->modelMatrix, ground->pos);
		sphere->modelMatrix = glm::translate(sphere->modelMatrix, sphere->pos);

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
		// 
		//��������
		memcpy(pFrame->ui_ubo.mapped, uiPts.data(), sizeof(vec4) * 6);
		memcpy(pFrame->view_ubo.mapped, &view->data, sizeof(View::UniformBufferObject));
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
			case 110:
			{
				//���ground �����
				pod.model = ground->modelMatrix;
				vkCmdPushConstants(cmd, piLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, size, &pod);
				ground->drawGeo(cmd);
			}
			break;
			case 111:
				//���ground �����
				pod.model = ground->modelMatrix;
				vkCmdPushConstants(cmd, piLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, size, &pod);
				ground->drawGeo(cmd);
				break;
			case 2:
				/* ���� */
				pod.model = sphere->modelMatrix;
				vkCmdPushConstants(cmd, piLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, size, &pod);
				sphere->drawGeo(cmd);
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
		sphere->clean();
	}
};
