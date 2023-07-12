#pragma once
#include <vector>
#include "glm.hpp"
#include "Geo.h"
#include "SimpleRandom.h"
#include "Frame.h"
#include "View.h"

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
	std::array<vec4, 6> uiPts;

	View* view;

	geos::Geo* ground;
	geos::GeoSphere* sphere;

	//创建 模型
	void prepare(VulkanDevice* vulkanDevice, VkExtent2D extent)
	{
		/* ui点 */
		uiPts[0] = { -0.95,-0.7f,0,1 };
		uiPts[1] = { -0.95,-0.95,0,0 };
		uiPts[2] = { -0.45f,-0.95,1,0 };

		uiPts[3] = { -0.45f,-0.7f,1,1 };
		uiPts[4] = uiPts[0];
		uiPts[5] = uiPts[2];
		/* 相机控制 */
		view = new View({ 0,0,1 }, 9.0f, { -3.0f,6.0f }, extent);
		ground = new geos::GeoPlane(8, { 0,0,-2 });
		ground->prepareBuffer(vulkanDevice);
		sphere = new geos::GeoSphere(0.6f, { 0.2f,0.2f }, 32, 24);
		sphere->prepareBuffer(vulkanDevice);

		Random rnd(0, { 0,1.0f });

		/* 设置物体的初始 位置-姿势 */
		ground->pos = { 0,0,0 };
		sphere->pos = { 0,0,1 };
		ground->modelMatrix = glm::translate(ground->modelMatrix, ground->pos);
		sphere->modelMatrix = glm::translate(sphere->modelMatrix, sphere->pos);

		/* 在水平面上 旋转模型 */
		geos::Geo* target = nullptr;
		if (nullptr != target)
		{
			//target->modelMatrix= glm::rotate(target->modelMatrix, glm::radians(68.0f), glm::vec3(0.0f, 0, 1.0f));
		}
	}
	/* 更新-UBO 资源 */
	void update(Frame* pFrame, float time, float deltaTime,int opKey)
	{
		view->update(deltaTime,opKey);
		// 
		//拷贝数据
		memcpy(pFrame->ui_ubo.mapped, uiPts.data(), sizeof(vec4) * 6);
		memcpy(pFrame->view_ubo.mapped, &view->data, sizeof(View::UniformBufferObject));
	}
	/*
	用idx 将 geo 以不同的方式渲染
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
				//最后画ground 故意的
				pod.model = ground->modelMatrix;
				vkCmdPushConstants(cmd, piLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, size, &pod);
				ground->drawGeo(cmd);
			}
			break;
			case 111:
				//最后画ground 故意的
				pod.model = ground->modelMatrix;
				vkCmdPushConstants(cmd, piLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, size, &pod);
				ground->drawGeo(cmd);
				break;
			case 2:
				/* 球体 */
				pod.model = sphere->modelMatrix;
				vkCmdPushConstants(cmd, piLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, size, &pod);
				sphere->drawGeo(cmd);
				break;

			case 3:
				//画 UI
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
