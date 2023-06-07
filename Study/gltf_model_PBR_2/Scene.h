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
#include "PbrEnv.h"

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
	geos::PbrBasic pbrBasic_bg = { {0.7f,0.6f,0.5f,1},0.7f,0.15f };		//roughtness,metallic

	PbrEnv* env;
	vks::gltfModel_pbr* helmet;
	vks::gltfModel_pbr* ship;
	vks::gltfModel_pbr* dinosaur;
	vks::gltfModel_pbr* landscape;

	geos::gltfPbrRender_spec* helmetRender;
	geos::gltfPbrRender_spec* dinoRender;
	geos::gltfPbrRender_spec* shipRender1;
	geos::gltfPbrRender_spec* shipRender2;

	geos::DebugPoints dxPoint;

	float deltaTime, timer = 0;
	bool displayShadowMap = true;
	int flipCounter_shadow = 3;		// 用于阴影的调试
	int flipCounter_equation = 6;	//保证第一次 切换到 1
	int flipCounter_viewInputs =7;	//保证第一次 切换到 1
	//创建 模型
	void prepare(VulkanDevice* vulkanDevice, VkExtent2D extent, Input* input, uint32_t swapchainImgCount)
	{
		this->vulkanDevice = vulkanDevice;
		this->input = input;
		/* 相机控制 */
		view = new View(extent);
		//gltf 模型信息
		vks::gltfModel_pbr::ModelInfo helmetInfo = {
			"../data/models/DamagedHelmet/glTF-Embedded/DamagedHelmet.gltf",
			1.6f,
			{0,3.5f,6}
		};
		vks::gltfModel_pbr::ModelInfo shipInfo = {
			"../models/ship.glb",
			3.0f,	{0,3,0},
			60,	{0,1,0}
		};
		vks::gltfModel_pbr::ModelInfo dinosaurInfo = {
			"../models/Rampaging T-Rex.glb",
			//"../models/Unicorn.glb",
			1.0f,
			{9,3.2f,0},
			15,	{0,1,0}
		};
		vks::gltfModel_pbr::ModelInfo landscapeInfo = {
			"../models/island.glb",
			1.0f,
			{0,0.0f,0}
		};

		//环境照明
		env = new PbrEnv(vulkanDevice,
			"../data/textures/empty.ktx",
			"../data/environments/papermill.ktx",
			"../data/models/Box/glTF-Embedded/Box.gltf");

		//baseColor+ metallicRoughness+ emissive+ normal+ oc
		helmet = new vks::gltfModel_pbr(vulkanDevice, swapchainImgCount,env, helmetInfo,true);	//只有一个 金属流材质
		//normal+oc+specGloss+diffuse
		ship = new vks::gltfModel_pbr(vulkanDevice, swapchainImgCount,env, shipInfo,true);		//2个材质
		dinosaur = new vks::gltfModel_pbr(vulkanDevice, swapchainImgCount,env, dinosaurInfo,true);
		//第二个材质 specGloss+diffuse		第三个材质 oc+specGloss+diffuse
		landscape = new vks::gltfModel_pbr(vulkanDevice, swapchainImgCount,env, landscapeInfo,true);


		geos::DebugPoints::Point pt;
		pt = { glm::mat4(1.0),view->lightPoses[0],{1,0,0,18} };
		dxPoint.addPoint(pt);
	}
	void prepareStep2(VkDescriptorPool descriptorPool, VkRenderPass renderPass,std::vector<Frame>* frames)
	{
		helmet->setup(descriptorPool);
		helmet->preparePipelines(renderPass);
		ship->setup(descriptorPool);
		ship->preparePipelines(renderPass);
		dinosaur->setup(descriptorPool);
		dinosaur->preparePipelines(renderPass);
		landscape->setup(descriptorPool);
		landscape->preparePipelines(renderPass);

		//获取渲染参数
		helmetRender = new geos::gltfPbrRender_spec();
		helmetRender->isMetallic = true;
		helmetRender->emptyImg = &env->empty.descriptor;
		helmetRender->mat.prefilteredCubeMipLevels = env->prefilteredCubeMipLevels;
		helmetRender->mat.isMetallic = 1;
		helmet->getSpecRender(helmetRender);

		dinoRender = new geos::gltfPbrRender_spec();
		dinoRender->isMetallic = false;
		dinoRender->emptyImg = &env->empty.descriptor;
		dinoRender->mat.prefilteredCubeMipLevels = env->prefilteredCubeMipLevels;
		dinoRender->mat.isMetallic = 0;
		dinosaur->getSpecRender(dinoRender);

		shipRender1 = new geos::gltfPbrRender_spec();
		shipRender1->isMetallic = false;
		shipRender1->emptyImg = &env->empty.descriptor;
		shipRender1->mat.prefilteredCubeMipLevels = env->prefilteredCubeMipLevels;
		shipRender1->mat.isMetallic = 0;
		ship->getSpecRender(shipRender1,0);

		shipRender2 = new geos::gltfPbrRender_spec();
		shipRender2->isMetallic = false;
		shipRender2->emptyImg = &env->empty.descriptor;
		shipRender2->mat.prefilteredCubeMipLevels = env->prefilteredCubeMipLevels;
		shipRender2->mat.isMetallic = 0;
		ship->getSpecRender(shipRender2,1);

		dxPoint.prepare(vulkanDevice, renderPass,true);
		//添加 一个albedo 用于着色//恐龙
		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			(*frames)[i].add_pbrEnv(vulkanDevice->logicalDevice, env);
			(*frames)[i].add_pbrRender(vulkanDevice->logicalDevice, helmetRender, 0);
			(*frames)[i].add_pbrRender(vulkanDevice->logicalDevice, dinoRender,2);
			(*frames)[i].add_pbrRender(vulkanDevice->logicalDevice, shipRender1, 1);
			(*frames)[i].add_pbrRender(vulkanDevice->logicalDevice, shipRender2, 11);
		}
	}
	/* 画一个 gltf 模型 */
	void draw_gltf(VkCommandBuffer cmd, uint32_t cmd_idx, int modelIdx)
	{
		vks::gltfModel_pbr* gltf = nullptr;
		switch (modelIdx)
		{
		case 0:
			gltf = helmet;
			break;
		case 1:
			gltf = ship;
			break;
		case 2:
			gltf = dinosaur;
			break;
		case 3:
			gltf = landscape;
			break;
		}
		vkglTF::Model& model = gltf->scene;
		VkDeviceSize offsets[1] = { 0 };
		/* 顶点，三角面数据 */
		vkCmdBindVertexBuffers(cmd, 0, 1, &model.vertices.buffer, offsets);
		if (model.indices.buffer != VK_NULL_HANDLE)
		{
			vkCmdBindIndexBuffer(cmd, model.indices.buffer, 0, VK_INDEX_TYPE_UINT32);
		}

		gltf->boundPipeline = VK_NULL_HANDLE;
		/* 渲染顺序 opaque-mask-blend */
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
	void draw_gltf_ByXPipe(VkCommandBuffer cmd, VkPipelineLayout pipeLayout, int modelIdx,int idxNode)
	{
		vks::gltfModel_pbr* gltf = nullptr;
		switch (modelIdx)
		{
		case 0:
			gltf = helmet;
			break;
		case 1:
			gltf = ship;
			break;
		case 2:
			gltf = dinosaur;
			break;
		case 3:
			gltf = landscape;
			break;
		}
		vkglTF::Model& model = gltf->scene;
		gltf->counter = 0;//记录渲染
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(cmd, 0, 1, &model.vertices.buffer, offsets);
		vkCmdBindIndexBuffer(cmd, model.indices.buffer, 0, VK_INDEX_TYPE_UINT32);
		VkShaderStageFlags stageVGF = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		gltf->boundPipeline = VK_NULL_HANDLE;
		for (auto node : model.nodes)
		{
			gltf->renderNode_ByXPipe(cmd, node, pipeLayout, stageVGF, vkglTF::Material::ALPHAMODE_OPAQUE, idxNode);
		}
	}
	void draw_points(VkCommandBuffer cmd)
	{
		glm::mat4  mvp = view->data.proj * view->data.view;
		dxPoint.draw(cmd, mvp);
	}
	/* 更新-UBO 资源 */
	void update(Frame* pFrame, uint32_t imageIndex, float time, float deltaTime)
	{
		if (input->flipEquation)
		{
			input->flipEquation = false;
			flipCounter_equation =(flipCounter_equation + 1) % 6;		//0::从 1 到 5

			helmetRender->mat.debugViewEquation = flipCounter_equation;
			dinoRender->mat.debugViewEquation = flipCounter_equation;
			shipRender1->mat.debugViewEquation = flipCounter_equation;
			shipRender2->mat.debugViewEquation = flipCounter_equation;

			helmetRender->mat.debugViewInputs = 0;
			dinoRender->mat.debugViewInputs = 0;
			shipRender1->mat.debugViewInputs = 0;
			shipRender2->mat.debugViewInputs = 0;
		}
		if (input->flipViewInputs)
		{
			input->flipViewInputs = false;
			flipCounter_viewInputs =  (flipCounter_viewInputs + 1) % 7;	//0::从 1 到 6

			helmetRender->mat.debugViewEquation = 0;
			dinoRender->mat.debugViewEquation = 0;
			shipRender1->mat.debugViewEquation = 0;
			shipRender2->mat.debugViewEquation = 0;

			helmetRender->mat.debugViewInputs = flipCounter_viewInputs;
			dinoRender->mat.debugViewInputs = flipCounter_viewInputs;
			shipRender1->mat.debugViewInputs = flipCounter_viewInputs;
			shipRender2->mat.debugViewInputs = flipCounter_viewInputs;
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
		helmet->updateUniformBuffers(imageIndex, view->data.proj, view->data.view, glm::vec3(view->data.camera));
		ship->updateUniformBuffers(imageIndex, view->data.proj, view->data.view, glm::vec3(view->data.camera));
		dinosaur->updateUniformBuffers(imageIndex, view->data.proj, view->data.view, glm::vec3(view->data.camera));
		landscape->updateUniformBuffers(imageIndex, view->data.proj, view->data.view, glm::vec3(view->data.camera));
		//拷贝数据
		memcpy(pFrame->ubo_scene.mapped, &view->data, sizeof(View::UniformBufferObject));
		memcpy(pFrame->ubo_pbr_bg.mapped, &pbrBasic_bg, sizeof(geos::PbrBasic));

		memcpy(pFrame->ubo_pbr_helmet.mapped, &helmetRender->mat, sizeof(geos::PbrMaterial));
		memcpy(pFrame->ubo_pbr_dino.mapped, &dinoRender->mat, sizeof(geos::PbrMaterial));
		memcpy(pFrame->ubo_pbr_ship1.mapped, &shipRender1->mat, sizeof(geos::PbrMaterial));
		memcpy(pFrame->ubo_pbr_ship2.mapped, &shipRender2->mat, sizeof(geos::PbrMaterial));
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

		env->clean();

		helmet->clean();
		ship->clean();
		dinosaur->clean();
		landscape->clean();

	}
};
