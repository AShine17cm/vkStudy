#pragma once
#include "constants.h"
#include <vector>
#include "glm.hpp"
#include "Geo.h"
#include "SimpleRandom.h"
#include "Frame.h"
#include "View.h"
#include "Input.h"

using namespace mg;

/*
将Geo 以不同的材质 渲染
*/
struct  Scene
{
	static const int countTextureArray = 8;

	/* 走 PushConstant的数据 */
	struct PerObjectData
	{
		glm::mat4 model;
		glm::vec4 texIndex;
	};


	View* view;
	Input* input;
	struct UIData
	{
		std::array<vec4, 6 * countUI> pts;				//用于在 近剪切平面上画 UI
		glm::ivec4 debug;
	}ui;
	
	std::vector<geos::GeoSquarePillar*> pillars;		//方柱，非实例化
	std::vector<geos::GeoCube*> cubes;					//测试 3D-Texture
	std::vector<geos::GeoCube*> cubes_ring;				//mip-maps
	std::vector<float> spds;

	geos::Geo* ground;
	geos::GeoCube* cube;
	geos::GeoSphere* sphere;

	float deltaTime;
	bool displayShadowMap = true;
	int flipCounter=3;// 用于阴影的调试
	//创建 模型
	void prepare(VulkanDevice* vulkanDevice, VkExtent2D extent, Input* input)
	{
		this->input = input;
		/* ui点 */
		ui.pts[0] = { -0.95,-0.7f,0,1 };
		ui.pts[1] = { -0.95,-0.95,0,0 };
		ui.pts[2] = { -0.45f,-0.95,1,0 };
		ui.pts[3] = { -0.45f,-0.7f,1,1 };
		ui.pts[4] = ui.pts[0];
		ui.pts[5] = ui.pts[2];
		
		ui.pts[6 + 0] = { 0.2f,-0.2f,0,1 };
		ui.pts[6 + 1] = { 0.2f,-1.0f,0,0 };
		ui.pts[6 + 2] = { 1.0f,-1.0f,1,0 };
		ui.pts[6 + 3] = { 1.0f,-0.2f,1,1 };
		ui.pts[6 + 4] = ui.pts[6 + 0];
		ui.pts[6 + 5] = ui.pts[6 + 2];
		ui.debug = { 1,1,1,1 };
		/* 相机控制 */
		view = new View({ 0,0,1 }, 9.0f, { -6.0f,6.0f }, extent);
		ground = new geos::GeoPlane(8, { 0,0,-2 });
		ground->prepareBuffer(vulkanDevice);
		sphere = new geos::GeoSphere(0.6f, { 0.2f,0.2f }, 32, 24);
		sphere->prepareBuffer(vulkanDevice);
		/* 立方体  将被Instancing的物体 */
		float size = 0.3f;
		vec3 clipA = { 0.5f,0.3f,0.2f };
		vec3 clipB = { 0.4f,0.2f,0.45f };
		cube = new geos::GeoCube(size, clipA, clipB);
		cube->prepareBuffer(vulkanDevice);

		/* 设置物体的初始 位置-姿势 */
		ground->pos = { 0,0,0 };
		sphere->pos = { 0,0,0.6f };
		cube->pos = { 0,0,2 };
		ground->modelMatrix = glm::translate(ground->modelMatrix, ground->pos);
		sphere->modelMatrix = glm::translate(sphere->modelMatrix, sphere->pos);
		cube->modelMatrix = glm::translate(cube->modelMatrix, cube->pos);

		/* 方柱 */
		vec2 baseSize = { 0.3f,0.4f };
		vec2 topSize = { 0.2f,0.25f };
		vec2 heights = { 0.8f,0.8f };   //x 是绝对值，y是比例
		vec2 topCenter = { 0.0f,0.05f };
		vec2 uvPlane = { 1.0f,1.0f };
		Random rnd(0, { 0,1.0f });

		float PI = glm::pi<float>();
		int countPillar =  15;
		float radius = 2.0f;
		float stepAng = PI * 2.0 / countPillar;
		float ang;
		/* 环形柱阵 */
		for (int i = 0; i < countPillar; i++)
		{
			ang = stepAng * i;
			/* 做一些随机 形状-半径-角度 */
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
		/* 一队 Cube */
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
		/* 环阵 初始化 */
		int countRing = 16;
		glm::mat4 idenity = glm::mat4(1.0f);
		radius = 1.6f;
		stepAng = PI*2.0f / countRing;
		float height = 0.6f;
		for (int i = 0; i < countRing; i++) 
		{
			float r = radius;// +rnd.generate() * 0.5f;
			ang = stepAng * i;
			float sin = r * glm::sin(ang);
			float cos = r * glm::cos(ang);

			glm::mat4 model=glm::translate(idenity, { cos,sin,height });
			ang = ang * (1.0f + (rnd.generate() - 0.5f) * 0.2f);
			model= glm::rotate(model, ang, { 0,0,1.0f });
			auto cube = new geos::GeoCube(size, clipA, clipB);
			cube->prepareBuffer(vulkanDevice);
			cube->pos = { cos,sin,height };
			cube->modelMatrix = model;
			cubes_ring.push_back(cube);
			spds.push_back(rnd.generate());
		}
	}
	/* 更新-UBO 资源 */
	void update(Frame* pFrame, float time, float deltaTime)
	{
		if (input->flipShadows) //3个光源的阴影,逐次展示，共同展示
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
		view->update(deltaTime,input->opKey);
		float ang = time * glm::radians(30.0f);//time是相对时间
		updateInstance(ang);
		//拷贝数据
		memcpy(pFrame->ubo_ui.mapped, &ui, sizeof(UIData));
		memcpy(pFrame->ubo_scene.mapped, &view->data, sizeof(View::UniformBufferObject));
	}
	void updateInstance(float offsetAng) 
	{
		int countRing = cubes_ring.size();
		float PI = glm::pi<float>();
		float stepAng = PI * 2.0 / countRing;
		float radius = 1.5f;
		float ang;
		float instanceH = 0.6f;
		glm::mat4 idenity = glm::mat4(1.0f);
		for (int i = 0; i < countRing; i++)
		{
			ang =offsetAng+ stepAng * i;

			float sin = radius * glm::sin(ang);
			float cos = radius * glm::cos(ang);
			auto cube = cubes_ring[i];
			glm::mat4 rt = glm::rotate(idenity, spds[i], { 1,0,0 });//自旋
			vec3 pos = { cos,sin,instanceH };
			glm::mat4 model = glm::translate(idenity,pos);
			model = glm::rotate(model, ang, { 0,0,1 });
			cube->modelMatrix= model*rt;
		}
	}
	/*
	用idx 将 geo 以不同的方式渲染
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
					/* 切换到Array的不同Layer */
					pod.texIndex = { i % 2,0,0,0 };
					vkCmdPushConstants(cmd, piLayout, stageVGF, 0, size, &pod);
					pillars[i]->drawGeo(cmd);
				}
			}
			break;
			case 1:
				/* Cube队列 3d-texture */
				for (uint32_t i = 0; i < cubes.size(); i++)
				{
					//glm::vec4 posVS =glm::vec4( cubes[i]->pos,1.0) * view->data.view;
					pod.model = cubes[i]->modelMatrix;
					pod.texIndex = { i,0,0,0 };
					vkCmdPushConstants(cmd, piLayout, stageVGF, 0, size, &pod);
					cubes[i]->drawGeo(cmd);
				}
				break;
			case 2:
				/* 运动的Cube环阵 */
				for (uint32_t i = 0; i < cubes_ring.size(); i++) 
				{
					pod.model = cubes_ring[i]->modelMatrix;
					pod.texIndex = { i % 2,0,0,0 };
					vkCmdPushConstants(cmd, piLayout, stageVGF, 0, size, &pod);
					cubes_ring[i]->drawGeo(cmd);
				}
				break;
			case 3:
				/* 球体 */
				pod.model = sphere->modelMatrix;
				vkCmdPushConstants(cmd, piLayout, stageVGF, 0, size, &pod);
				sphere->drawGeo(cmd);
				break;
			case 4:
				/* texture */
				pod.model = ground->modelMatrix;
				vkCmdPushConstants(cmd, piLayout, stageVGF, 0, size, &pod);
				ground->drawGeo(cmd);
				break;
			case 5:
				//画 UI
				firstPt = 0;
				pod.texIndex = { 0,0,0,0 };
				vkCmdPushConstants(cmd, piLayout, stageVGF, 0, size, &pod);
				vkCmdDraw(cmd, 6, 1, firstPt, 0);
				break;
			case 6:
				//展示 阴影的Map
				if (input->displayShadowmap)
				{
					pod.texIndex = { 1,0,0,0 };
					firstPt = 6;
					vkCmdPushConstants(cmd, piLayout, stageVGF, 0, size, &pod);
					vkCmdDraw(cmd, 6, 1, firstPt, 0);
				}
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
		for (uint32_t i = 0; i < cubes_ring.size(); i++)
		{
			cubes_ring[i]->clean();
		}
	}
};
