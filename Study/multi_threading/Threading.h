#pragma once
#include "constants.h"
#include <vector>
#include "glm.hpp"
#include "Geo.h"
#include "SimpleRandom.h"
#include "Frame.h"
#include "View.h"
#include "Input.h"
#include "SimpleRandom.h"
#include "batches.h"
#include "Geo.h"

#include "threadpool.hpp"

using namespace mg;

/*	主线程中 一个 Primary 一个 Secondary CommandBuffer
	工作线程上只有 Secondary 等级的 CommandBuffer
*/
struct Threading
{
	VkDevice device;
	VkCommandPool cmdPool;
	PipelineHub* pipes;

	struct ObjectData {
		glm::mat4 model;
		glm::vec3 pos;
		glm::vec3 rotation;
		float rotationDir;
		float rotationSpeed;
		float scale;
		float deltaT;
		float stateT = 0;
	};
	struct ThreadPushData
	{
		glm::mat4 mvp;
		glm::vec4 data;
	};
	struct ThreadData
	{
		VkPipeline pipeline;
		VkPipelineLayout pi_Layout;
		geos::Geo* geo;

		VkCommandPool cmdPool;
		std::vector<VkCommandBuffer> cmds;					// One command buffer per render object
		std::vector<ThreadPushData> pushConstBlock;			// One push constant block per render object
		std::vector<ObjectData> objectDatas;				// Per object information (position, rotation, etc.)
	};
	std::vector<ThreadData> threadDatas;
	std::vector<geos::Geo*> geos;
	//std::vector<VkDescriptorSet> descriptorSets;

	ThreadPool threadPool;
	// Fence to wait for all command buffers to finish before
	// presenting to the swap chain
	VkFence renderFence = {};

	//VkCommandBuffer cmd_primary;
	VkCommandBuffer cmd_secondary;

	Threading(VkDevice device, VkCommandPool cmdPool, PipelineHub* pipes)
	{
		this->device = device;
		this->cmdPool = cmdPool;
		this->pipes = pipes;
	}

	/*	创建主线程上的 Primary+Secondary cmd
		创建工作线程上的 CommandBufferPool
		创建工作线程上的 Secondary cmd */
	void prepare_MultiThread_Renderer(VulkanDevice* vulkanDevice,Frame* pframe)
	{
		/* 准备一些几何体  */
		float size = 0.3f;
		vec3 clipA = { 0.5f,0.3f,0.2f };
		vec3 clipB = { 0.4f,0.2f,0.45f };
		geos::GeoCube* cube = new geos::GeoCube(size, clipA, clipB);
		cube->prepareBuffer(vulkanDevice);
		geos::GeoSphere* sphere = new geos::GeoSphere(0.6f, { 0.2f,0.2f }, 32, 24);
		sphere->prepareBuffer(vulkanDevice);
		geos.push_back(cube);
		geos.push_back(sphere);
		
		//descriptorSets.push_back(pframe->tex_3d);

		/* 主线程上的 cmd */
		VkCommandBufferAllocateInfo cmdAI = {};
		cmdAI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdAI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdAI.commandPool = cmdPool;
		cmdAI.commandBufferCount = 1;

		//MG_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdAI, &cmd_primary));

		cmdAI.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		MG_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdAI, &cmd_secondary));

		threadDatas.resize(NUM_THREADS);
		float maxX = std::floor(std::sqrt(NUM_THREADS * NUM_CMD_PER_THREAD * NUM_PER_CMD));

		//创建 NUM_THREADS 个线程
		for (uint32_t i = 0; i < NUM_THREADS; i++)
		{
			ThreadData* thread = &threadDatas[i];
			thread->geo = geos[i % geos.size()];
			thread->pipeline = pipes->pi_Tex;
			thread->pi_Layout = pipes->piLayout_solid;

			//线程: 每个线程一个 Command Buffer Pool
			VkCommandPoolCreateInfo poolCI{};
			poolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolCI.queueFamilyIndex = vulkanDevice->queueFamilyIndices.graphics;
			poolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			MG_CHECK_RESULT(vkCreateCommandPool(device, &poolCI, nullptr, &thread->cmdPool));

			//线程: 创建 NUM_CMD_PER_THREAD 个 Command Buffer:Secondary
			thread->cmds.resize(NUM_CMD_PER_THREAD);
			VkCommandBufferAllocateInfo cmdAI_second{};
			cmdAI_second.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmdAI_second.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;//次级的 cmd
			cmdAI_second.commandPool = thread->cmdPool;
			cmdAI_second.commandBufferCount = NUM_CMD_PER_THREAD;
			MG_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdAI_second, thread->cmds.data()));

			thread->pushConstBlock.resize(NUM_CMD_PER_THREAD * NUM_PER_CMD);
			thread->objectDatas.resize(NUM_CMD_PER_THREAD * NUM_PER_CMD);
			//每个线程上 N*M 个物体的位置数据
			for (uint32_t k = 0; k < NUM_CMD_PER_THREAD * NUM_PER_CMD; k++)
			{
				float theta = 2.0f * float(3.1415926) * rnd(1.0f);
				float phi = acos(1.0f - 2.0f * rnd(1.0f));
				thread->objectDatas[k].pos = glm::vec3(sin(phi) * cos(theta), 0.0f, cos(phi)) * 35.0f;
				thread->objectDatas[k].rotation = glm::vec3(0.0f, rnd(360.0f), 0.0f);
				thread->objectDatas[k].deltaT = rnd(1.0f);
				thread->objectDatas[k].rotationDir = (rnd(100.0f) < 50.0f) ? 1.0f : -1.0f;
				thread->objectDatas[k].rotationSpeed = (2.0f + rnd(4.0f)) * thread->objectDatas[k].rotationDir;
				thread->objectDatas[k].scale = 0.75f + rnd(0.5f);



				thread->pushConstBlock[k].data = glm::vec4(rnd(1.0f), rnd(1.0f), rnd(1.0f), 1.0f);
			}
		}
	}
	/*
		在工作线程上 记录 cmd
		不需要 BeginRenderPass 绑定 FrameBuffer
		cmdBI的flags是 Render_Pass_Continue
	*/
	void thread_RenderCode(uint32_t threadIdx, uint32_t cmdIdx, VkCommandBufferInheritanceInfo inheritanceInfo)
	{
		ThreadData* thread = &threadDatas[threadIdx];
		ObjectData* objectData = &thread->objectDatas[cmdIdx];

		VkCommandBufferBeginInfo cmdBI{};
		cmdBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBI.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;//Render_Pass_Continue
		cmdBI.pInheritanceInfo = &inheritanceInfo;

		VkCommandBuffer cmd = thread->cmds[cmdIdx];
		MG_CHECK_RESULT(vkBeginCommandBuffer(cmd, &cmdBI));
		/* 不需要 BeginRenderPass 绑定 FrameBuffer */
		mg::batches::SetViewport(cmd, { WIDTH,HEIGHT });
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, thread->pipeline);
		/* 每个 cmd 渲染 NUM_PER_CMD 个物体 */
		for (int k = 0; k < NUM_PER_CMD; k++) 
		{
			vkCmdPushConstants(cmd,
				thread->pi_Layout,
				VK_SHADER_STAGE_VERTEX_BIT, 0,
				sizeof(ThreadPushData),
				&thread->pushConstBlock[cmdIdx * NUM_PER_CMD + k]);

			thread->geo->drawGeo(cmd, 1);
		}

		MG_CHECK_RESULT(vkEndCommandBuffer(cmd));
	}
	float rnd(float range)
	{
		static std::default_random_engine rndEngine;
		rndEngine.seed(0);

		std::uniform_real_distribution<float> rndDist(0.0, range);
		return rndDist(rndEngine);
	}
};