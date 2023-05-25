#pragma once
#include "constants.h"
#include "commonData.h"
#include <vector>
#include "glm.hpp"
#include "Geo.h"
#include "SimpleRandom.h"
#include "Frame.h"
#include "View.h"
#include "Input.h"

#include "VulkanglTFModel.h"
#include "gltfModel_pbr.h"

using namespace mg;

/*
��Geo �Բ�ͬ�Ĳ��� ��Ⱦ
*/
struct  Scene
{
	static const int countTextureArray = 8;
	const float instanceH = 1.3f;
	VulkanDevice* vulkanDevice;
	View* view;
	Input* input;
	struct UIData
	{
		std::array<vec4, 6 * countUI> pts;				//������ ������ƽ���ϻ� UI
		glm::ivec4 debug;
	}ui;

	std::vector<geos::GeoSquarePillar*> pillars;		//��������ʵ����
	std::vector<geos::GeoCube*> cubes_ring;				//mip-maps
	std::vector<float> spds;

	geos::Geo* ground;
	geos::GeoCube* cube;

	vks::gltfModel_pbr* gltf;

	float rotateRadius = 2.2f;

	float deltaTime, timer = 0;
	bool displayShadowMap = true;
	int flipCounter = 3;// ������Ӱ�ĵ���
	//���� ģ��
	void prepare(VulkanDevice* vulkanDevice, VkExtent2D extent, Input* input,uint32_t swapchainImgCount)
	{
		this->vulkanDevice = vulkanDevice;
		this->input = input;
		/* ui�� */
		ui.pts[0] = { -0.95,-0.7f,0,1 };
		ui.pts[1] = { -0.45f,-0.95,1,0 };
		ui.pts[2] = { -0.95,-0.95,0,0 };
		ui.pts[3] = { -0.45f,-0.7f,1,1 };
		ui.pts[4] = ui.pts[1];
		ui.pts[5] = ui.pts[0];

		ui.pts[6 + 0] = { 0.2f,-0.2f,0,1 };
		ui.pts[6 + 1] = { 1.0f,-1.0f,1,0 };
		ui.pts[6 + 2] = { 0.2f,-1.0f,0,0 };
		ui.pts[6 + 3] = { 1.0f,-0.2f,1,1 };
		ui.pts[6 + 4] = ui.pts[6 + 1];
		ui.pts[6 + 5] = ui.pts[6 + 0];
		ui.debug = { 1,1,1,1 };
		/* ������� */
		view = new View({ 0,0,0 }, 9.0f, { -6.0f,6.0f }, extent);
		ground = new geos::GeoPlane(8, { 0,-2,0 });
		ground->prepareBuffer(vulkanDevice);
		/* ������  ����Instancing������ */
		float size = 0.3f;
		vec3 clipA = { 0.5f,0.3f,0.2f };
		vec3 clipB = { 0.4f,0.2f,0.45f };
		cube = new geos::GeoCube(size, clipA, clipB);
		cube->prepareBuffer(vulkanDevice);

		float PI = glm::pi<float>();
		/* ��������ĳ�ʼ λ��-���� */
		ground->pos = { 0,1.6f,0 };
		cube->pos = { 0,0,2 };
		ground->modelMatrix = glm::translate(ground->modelMatrix, ground->pos);
		ground->modelMatrix = glm::rotate(ground->modelMatrix, PI * 0.5f, { 1.0f,0.0f,0.0f });
		cube->modelMatrix = glm::translate(cube->modelMatrix, cube->pos);

		/* ���� */
		vec2 baseSize = { 0.3f,0.4f };
		vec2 topSize = { 0.2f,0.25f };
		vec2 heights = { 0.8f,0.8f };   //x �Ǿ���ֵ��y�Ǳ���
		vec2 topCenter = { 0.0f,0.05f };
		vec2 uvPlane = { 1.0f,1.0f };
		Random rnd(0, { 0,1.0f });

		int countPillar = 15;
		float radius = 3.0f;
		float stepAng = PI * 2.0 / countPillar;
		float ang;
		float h = 1.6f;
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
			pillar->pos = { cos,h,sin };
			pillar->modelMatrix = glm::translate(pillar->modelMatrix, pillar->pos);
			pillar->modelMatrix= glm::rotate(pillar->modelMatrix, PI*0.5f, { 1.0f,0.0f,0.0f });//��ת90 ��xzƽ�����
			pillar->modelMatrix = glm::rotate(pillar->modelMatrix, ang, { 0,0,1.0f });
			pillars.push_back(pillar);
		}
		/* ���� ��ʼ�� */
		int countRing = 16;
		glm::mat4 idenity = glm::mat4(1.0f);
		stepAng = PI * 2.0f / countRing;
		for (int i = 0; i < countRing; i++)
		{
			float r = rotateRadius;// +rnd.generate() * 0.5f;
			ang = stepAng * i;
			float sin = r * glm::sin(ang);
			float cos = r * glm::cos(ang);

			glm::mat4 model = glm::translate(idenity, { cos,sin,instanceH });
			ang = ang * (1.0f + (rnd.generate() - 0.5f) * 0.2f);
			model = glm::rotate(model, ang, { 0,0,1.0f });
			auto cube = new geos::GeoCube(size, clipA, clipB);
			cube->prepareBuffer(vulkanDevice);
			cube->pos = { cos,instanceH,sin };
			cube->modelMatrix = model;
			cubes_ring.push_back(cube);
			spds.push_back(rnd.generate());
		}
		gltf = new vks::gltfModel_pbr(vulkanDevice,swapchainImgCount);
	}

	/* ��һ�� gltf ģ�� */
	void draw_gltf(VkCommandBuffer cmd,uint32_t cmd_idx)
	{
		VkDeviceSize offsets[1] = { 0 };
		vkglTF::Model& model = gltf->models.scene;
		/* ���㣬���������� */
		vkCmdBindVertexBuffers(cmd, 0, 1, &model.vertices.buffer, offsets);
		if (model.indices.buffer != VK_NULL_HANDLE) 
		{
			vkCmdBindIndexBuffer(cmd, model.indices.buffer, 0, VK_INDEX_TYPE_UINT32);
		}

		gltf->boundPipeline = VK_NULL_HANDLE;
		/* ��Ⱦ˳�� opaque-mask-blend */
		for (auto node : model.nodes)
		{
			gltf->renderNode(cmd, node, cmd_idx, vkglTF::Material::ALPHAMODE_OPAQUE);
		}
		for (auto node : model.nodes)
		{
			gltf->renderNode(cmd,node, cmd_idx, vkglTF::Material::ALPHAMODE_MASK);
		}
		for (auto node : model.nodes)
		{
			gltf->renderNode(cmd,node, cmd_idx, vkglTF::Material::ALPHAMODE_BLEND);
		}
	}
	/* ����-UBO ��Դ */
	void update(Frame* pFrame,uint32_t imageIndex, float time, float deltaTime)
	{
		if (input->flipShadows) //3����Դ����Ӱ,���չʾ����ͬչʾ
		{
			input->flipShadows = false;
			flipCounter = (flipCounter + 1) % 4;
			switch (flipCounter)
			{
			case 0:
				ui.debug = { 1,0,0,0 };
				break;
			case 1:
				ui.debug = { 0,1,0,0 };
				break;
			case 2:
				ui.debug = { 0,0,1,0 };
				break;
			case 3:
				ui.debug = { 1,1,1,0 };
				break;
			}
			view->data.debug = ui.debug;
		}
		this->deltaTime = deltaTime;
		this->timer += deltaTime;
		view->update(deltaTime, input->opKey);
		float ang = time * glm::radians(30.0f);//time�����ʱ��
		updateInstance(ang);
		gltf->updateUniformBuffers(imageIndex, view->data.proj, view->data.view, glm::vec3(view->data.camera));
		//��������
		memcpy(pFrame->ubo_ui.mapped, &ui, sizeof(UIData));
		memcpy(pFrame->ubo_scene.mapped, &view->data, sizeof(View::UniformBufferObject));
	}
	void updateInstance(float offsetAng)
	{
		int countRing = cubes_ring.size();
		float PI = glm::pi<float>();
		float stepAng = PI * 2.0 / countRing;
		//һ��ƽ�����˶��뾶����
		float band = 0.8f;
		float k = timer - (static_cast<int> (timer / 4.0f)) * 4.0f;
		k =glm::abs( k / 4.0f-0.5f)*2.0f;
		k = 3.0f * k * k - 2.0f * k * k * k;//(0,1)
		k = k - 0.5f;

		float radius = rotateRadius + k * band;
		float ang;
		glm::mat4 idenity = glm::mat4(1.0f);
		for (int i = 0; i < countRing; i++)
		{
			ang = offsetAng + stepAng * i;

			float sin = radius * glm::sin(ang);
			float cos = radius * glm::cos(ang);
			auto cube = cubes_ring[i];
			glm::mat4 rt = glm::rotate(idenity, spds[i], { 1,0,0 });//����
			vec3 pos = {cos,instanceH ,sin };
			glm::mat4 model = glm::translate(idenity, pos);
			model = glm::rotate(model, ang, { 0,0,1 });
			cube->modelMatrix = model * rt;
		}
	}
	/*
	��idx �� geo �Բ�ͬ�ķ�ʽ��Ⱦ
	*/
	void draw(VkCommandBuffer cmd, VkPipelineLayout piLayout, int batchIdx)
	{
		VkShaderStageFlags stageVGF = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		PerObjectData pod = { glm::mat4(1),{0,0,0,0} };
		uint32_t size = sizeof(PerObjectData);
		VkDeviceSize offsets[] = { 0 };
		uint32_t firstPt = 0;
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
				vkCmdPushConstants(cmd, piLayout, stageVGF, 0, size, &pod);
				pillars[i]->drawGeo(cmd);
			}
		}
		break;
		case 1:
			/* Cube���� 3d-texture */
			//draw_gltf(cmd);
			break;
		case 2:
			/* �˶���Cube���� */
			for (uint32_t i = 0; i < cubes_ring.size(); i++)
			{
				pod.model = cubes_ring[i]->modelMatrix;
				pod.texIndex = { i % 2,0,0,0 };
				vkCmdPushConstants(cmd, piLayout, stageVGF, 0, size, &pod);
				cubes_ring[i]->drawGeo(cmd);
			}
			break;
		case 3:
			/* ���� */
			//draw_gltf(cmd);
			break;
		case 4:
			/* texture */
			pod.model = ground->modelMatrix;
			vkCmdPushConstants(cmd, piLayout, stageVGF, 0, size, &pod);
			ground->drawGeo(cmd);
			break;
		case 5:
			//�� UI
			firstPt = 0;
			vkCmdDraw(cmd, 6, 1, firstPt, 0);
			break;
		case 6:
			//չʾ ��Ӱ��Map
			if (input->displayShadowmap)
			{
				firstPt = 6;
				vkCmdDraw(cmd, 6, 1, firstPt, 0);
			}
			break;
		}
	}

	void cleanup()
	{
		gltf->clean();

		ground->clean();
		cube->clean();
		for (uint32_t i = 0; i < pillars.size(); i++)
		{
			pillars[i]->clean();
		}
		for (uint32_t i = 0; i < cubes_ring.size(); i++)
		{
			cubes_ring[i]->clean();
		}
	}
};
