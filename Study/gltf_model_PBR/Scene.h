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

#include "VulkanglTFModel.h"
#include "gltfModel_pbr.h"

#include "DebugPoints.h"

using namespace mg;

/*
��Geo �Բ�ͬ�Ĳ��� ��Ⱦ
	camera ��transition ʱ z��ȡ��
	shader �� y ��ȡ��
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
	geos::PbrBasic pbrBasic = { {1,1,1,1},0.6f,0.3f };					//roughtness,metallic
	geos::PbrBasic pbrBasic_bg = { {0.6f,0.4f,0.2f,0},0.95f,0.05f };	//roughtness,metallic

	vks::gltfModel_pbr* helmet;
	vks::gltfModel_pbr* armor;
	vks::gltfModel_pbr* dinosaur;
	vks::gltfModel_pbr* landscape;

	geos::DebugPoints dxPoint;

	float rotateRadius = 4.2f;

	float deltaTime, timer = 0;
	bool displayShadowMap = true;
	int flipCounter = 3;// ������Ӱ�ĵ���
	//���� ģ��
	void prepare(VulkanDevice* vulkanDevice, VkExtent2D extent, Input* input, uint32_t swapchainImgCount)
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

		ui.debug = { 0,1,0,1 };
		/* ������� */
		view = new View(extent);
		//gltf ģ����Ϣ
		vks::gltfModel_pbr::ModelInfo helmetInfo = {
			"../data/models/DamagedHelmet/glTF-Embedded/DamagedHelmet.gltf",
			"../data/models/Box/glTF-Embedded/Box.gltf",
			"../data/textures/empty.ktx",
			"../data/environments/papermill.ktx",
			1.6f,
			{0,3.5f,6}
		};
		vks::gltfModel_pbr::ModelInfo armorInfo = {
			"../models/ship.glb",
			"../models/deferred_box.gltf",
			"../data/textures/empty.ktx",
			"../data/environments/papermill.ktx",
			3.0f,
			{0,3,0},
			60,
			{0,1,0}
		};
		vks::gltfModel_pbr::ModelInfo dinosaurInfo = {
			"../models/Rampaging T-Rex.glb",
			"../models/deferred_box.gltf",
			"../data/textures/empty.ktx",
			"../data/environments/papermill.ktx",
			1.0f,
			{9,3.2f,0}
		};
		vks::gltfModel_pbr::ModelInfo landscapeInfo = {
			"../models/island.glb",
			"../models/deferred_box.gltf",
			"../data/textures/empty.ktx",
			"../data/environments/papermill.ktx",
			1.0f,
			{0,0.0f,0}
		};
		helmet = new vks::gltfModel_pbr(vulkanDevice, swapchainImgCount, helmetInfo);
		armor = new vks::gltfModel_pbr(vulkanDevice, swapchainImgCount, armorInfo);
		dinosaur = new vks::gltfModel_pbr(vulkanDevice, swapchainImgCount, dinosaurInfo);
		landscape = new vks::gltfModel_pbr(vulkanDevice, swapchainImgCount, landscapeInfo);

		geos::DebugPoints::Point pt;
		pt = { glm::mat4(1.0),view->lightPoses[0],{1,0,0,18} };
		dxPoint.addPoint(pt);
		pt = { glm::mat4(1.0),view->lightPoses[1],{0,1,0,18} };
		dxPoint.addPoint(pt);
		pt = { glm::mat4(1.0),view->lightPoses[2],{0,0,1,18} };
		dxPoint.addPoint(pt);
	}
	void prepareStep2(VkDescriptorPool descriptorPool, VkRenderPass renderPass)
	{
		helmet->setup(descriptorPool);
		helmet->preparePipelines(renderPass);
		armor->setup(descriptorPool);
		armor->preparePipelines(renderPass);
		dinosaur->setup(descriptorPool);
		dinosaur->preparePipelines(renderPass);
		landscape->setup(descriptorPool);
		landscape->preparePipelines(renderPass);

		dxPoint.prepare(vulkanDevice, renderPass);
	}
	/* ��һ�� gltf ģ�� */
	void draw_gltf(VkCommandBuffer cmd, uint32_t cmd_idx, int modelIdx)
	{
		vks::gltfModel_pbr* gltf = nullptr;
		switch (modelIdx)
		{
		case 0:
			gltf = helmet;
			break;
		case 1:
			gltf = armor;
			break;
		case 2:
			gltf = dinosaur;
			break;
		case 3:
			gltf = landscape;
			break;
		}
		vkglTF::Model& model = gltf->models.scene;
		VkDeviceSize offsets[1] = { 0 };
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
			gltf->renderNode(cmd, node, cmd_idx, vkglTF::Material::ALPHAMODE_MASK);
		}
		for (auto node : model.nodes)
		{
			gltf->renderNode(cmd, node, cmd_idx, vkglTF::Material::ALPHAMODE_BLEND);
		}
	}
	void draw_gltf_ByXPipe(VkCommandBuffer cmd, VkPipelineLayout pipeLayout, int modelIdx)
	{
		vks::gltfModel_pbr* gltf = nullptr;
		switch (modelIdx)
		{
		case 0:
			gltf = helmet;
			break;
		case 1:
			gltf = armor;
			break;
		case 2:
			gltf = dinosaur;
			break;
		case 3:
			gltf = landscape;
			break;
		}

		vkglTF::Model& model = gltf->models.scene;
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(cmd, 0, 1, &model.vertices.buffer, offsets);
		vkCmdBindIndexBuffer(cmd, model.indices.buffer, 0, VK_INDEX_TYPE_UINT32);
		VkShaderStageFlags stageVGF = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		gltf->boundPipeline = VK_NULL_HANDLE;
		for (auto node : model.nodes)
		{
			gltf->renderNode_ByXPipe(cmd, node, pipeLayout, stageVGF, vkglTF::Material::ALPHAMODE_OPAQUE);
		}
	}
	void draw_points(VkCommandBuffer cmd)
	{
		glm::mat4  mvp = view->data.proj * view->data.view;
		dxPoint.draw(cmd, mvp);
	}
	/* ����-UBO ��Դ */
	void update(Frame* pFrame, uint32_t imageIndex, float time, float deltaTime)
	{
		if (input->flipShadows&&false) //3����Դ����Ӱ,���չʾ����ͬչʾ
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
		view->update(input, deltaTime);
		helmet->updateUniformBuffers(imageIndex, view->data.proj, view->data.view, glm::vec3(view->data.camera));
		armor->updateUniformBuffers(imageIndex, view->data.proj, view->data.view, glm::vec3(view->data.camera));
		dinosaur->updateUniformBuffers(imageIndex, view->data.proj, view->data.view, glm::vec3(view->data.camera));
		landscape->updateUniformBuffers(imageIndex, view->data.proj, view->data.view, glm::vec3(view->data.camera));
		//��������
		memcpy(pFrame->ubo_ui.mapped, &ui, sizeof(UIData));
		memcpy(pFrame->ubo_scene.mapped, &view->data, sizeof(View::UniformBufferObject));
		memcpy(pFrame->ubo_pbr.mapped, &pbrBasic, sizeof(geos::PbrBasic));
		memcpy(pFrame->ubo_pbr_bg.mapped, &pbrBasic_bg, sizeof(geos::PbrBasic));
	}
	/*
	��idx �� geo �Բ�ͬ�ķ�ʽ��Ⱦ
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
			//�� UI
			firstPt = 0;
			vkCmdDraw(cmd, 6, 1, firstPt, 0);
			break;
		case 6:
			//չʾ ��Ӱ��Map
			//if (input->displayShadowmap)
			//{
			//	firstPt = 6;
			//	vkCmdDraw(cmd, 6, 1, firstPt, 0);
			//}
			break;
		}
	}

	void cleanup()
	{
		dxPoint.clean();

		helmet->clean();
		armor->clean();
		dinosaur->clean();
		landscape->clean();
	}
};
