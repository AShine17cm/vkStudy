#pragma once
#include "constants.h"
#include "PerObjectData.h"
#include <vector>
#include "glm.hpp"
#include "Geo.h"
#include "SimpleRandom.h"
#include "Frame.h"
#include "View.h"
#include "Input.h"
#include "ui.h"

#include "DebugPoints.h"

using namespace mg;

/*
	将Geo 以不同的材质 渲染
	camera 做transition 时 z轴取反
	shader 里 y 轴取反
*/
struct  Scene
{
	static const int countTextureArray = 8;
	const float instanceH = 1.3f;
	VulkanDevice* vulkanDevice;
	View* view;
	Input* input;
	UIData* uiData;

	geos::DebugPoints dxPoint;

	float deltaTime, timer = 0;
	bool displayShadowMap = true;
	int flipCounter_shadow = 3;		// 用于阴影的调试
	//创建 模型
	void prepare(VulkanDevice* vulkanDevice, VkExtent2D extent, Input* input, uint32_t swapchainImgCount)
	{
		this->vulkanDevice = vulkanDevice;
		this->input = input;
		/* 相机控制 */
		view = new View(extent);

		geos::DebugPoints::Point pt;
		pt = { glm::mat4(1.0),view->lightPoses[0],{1,0,0,18} };
		dxPoint.addPoint(pt);
	}
	void prepareStep2(VkDescriptorPool descriptorPool, VkRenderPass renderPass,std::vector<Frame>* frames, UIData* uiData)
	{
		this->uiData = uiData;


		dxPoint.prepare(vulkanDevice, renderPass,true);
	}

	void draw_points(VkCommandBuffer cmd)
	{
		glm::mat4  mvp = view->data.proj * view->data.view;
		dxPoint.draw(cmd, mvp);
	}
	/* 更新-UBO 资源 */
	void update(Frame* pFrame, uint32_t imageIndex, float time, float deltaTime)
	{

		if (uiData->operate[0])
		{

		}

		if (input->flipShadows) //3个光源的阴影,逐次展示，共同展示
		{
			input->flipShadows = false;
			flipCounter_shadow = (flipCounter_shadow + 1) % 4;
			view->data.debug = { 1,1,1,1 };//这里不起作用
		}
		this->deltaTime = deltaTime;
		this->timer += deltaTime;
		view->update(input, deltaTime);

		//拷贝数据
		memcpy(pFrame->ubo_scene.mapped, &view->data, sizeof(View::UniformBufferObject));
	}
	/*
	用idx 将 geo 以不同的方式渲染
	*/
	void draw(VkCommandBuffer cmd, VkPipelineLayout piLayout, int batchIdx)
	{
		if (batchIdx == -1) draw_points(cmd);

		VkShaderStageFlags stageVGF = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		geos::PerObjectData pod = { glm::mat4(1),{0,0,0,0} };
		uint32_t size = sizeof(geos::PerObjectData);
		VkDeviceSize offsets[] = { 0 };
		uint32_t firstPt = 0;
		switch (batchIdx)
		{
		case 0:
			break;
		case 1:
			break;
		case 5:
			//画 UI
			firstPt = 0;
			vkCmdDraw(cmd, 6, 1, firstPt, 0);
			break;
		case 6:
			break;
		}
	}

	void cleanup()
	{
		dxPoint.clean();

	}
};
