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
	PipelineHub* pipes;

	ThreadPool threadPool;

	struct ThreadPushData
	{
		glm::mat4 model; //object to world
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
	};
	std::vector<ThreadData> threadDatas;
	std::vector<geos::Geo*> geos;

	std::default_random_engine rndEngine;			//用于 生成位置
	
	//ThreadPool threadPool;
	// Fence to wait for all command buffers to finish before
	// presenting to the swap chain
	//VkFence renderFence = {};

	Threading(VkDevice device, PipelineHub* pipes)
	{
		this->device = device;
		this->pipes = pipes;

		threadPool.setThreadCount(NUM_THREADS);
	}
	~Threading()
	{
		for (int i = 0; i < geos.size(); i++)
		{
			geos[i]->clean();
		}
		for (int i = 0; i < threadDatas.size(); i++) 
		{
			ThreadData td = threadDatas[i];
			vkFreeCommandBuffers(device, td.cmdPool, td.cmds.size(), td.cmds.data());
			vkDestroyCommandPool(device, td.cmdPool, nullptr);
		}
	}
	/*	创建主线程上的 Primary+Secondary cmd
		创建工作线程上的 CommandBufferPool
		创建工作线程上的 Secondary cmd */
	void prepare_MultiThread_Renderer(VulkanDevice* vulkanDevice)
	{
		/* 准备一些几何体  */
		float size = 0.3f;
		vec3 clipA = { 0.5f,0.3f,0.2f };
		vec3 clipB = { 0.4f,0.2f,0.45f };
		/* 方柱 */
		vec2 baseSize = { 0.3f,0.4f };
		vec2 topSize = { 0.2f,0.25f };
		vec2 heights = { 0.8f,0.8f };   //x 是绝对值，y是比例
		vec2 topCenter = { 0.0f,0.05f };
		vec2 uvPlane = { 1.0f,1.0f };

		geos::GeoCube* cube = new geos::GeoCube(size, clipA, clipB);
		cube->prepareBuffer(vulkanDevice);
		auto  pillar = new geos::GeoSquarePillar(baseSize, topSize, topCenter, heights, uvPlane);
		pillar->prepareBuffer(vulkanDevice);
		geos.push_back(cube);
		geos.push_back(pillar);
		
		threadDatas.resize(NUM_THREADS);

		float ring = 5.0f;
		float band = 18.0f;
		rndEngine.seed(0);
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
			cmdAI_second.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;//次级的 cmd
			cmdAI_second.commandPool = thread->cmdPool;
			cmdAI_second.commandBufferCount = NUM_CMD_PER_THREAD;
			MG_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdAI_second, thread->cmds.data()));

			thread->pushConstBlock.resize(NUM_CMD_PER_THREAD * NUM_OBJECT_PER_CMD);
			//每个线程上 N*M 个物体的位置数据
			for (uint32_t k = 0; k < NUM_CMD_PER_THREAD * NUM_OBJECT_PER_CMD; k++)
			{
				float phi =  rnd(2.0f)*3.1415926f;
				float R = ring + rnd(1.0f) * band;
				glm::vec3 pos = glm::vec3(sin(phi)*R, cos(phi)*R, -0.1f*R);
				float rotation = rnd(360.0f);
				float scale = 0.75f + rnd(0.5f);

				//物体的矩阵
				glm::mat4 model = glm::mat4(1.0f);
				model= glm::translate(model, pos);
				model = glm::rotate(model, rotation, {0,1,0});
				model = glm::rotate(model, rotation * 0.5f, { 1,0,0.5f });
				model = glm::scale(model,glm::vec3(scale));

				thread->pushConstBlock[k].model = model;
				thread->pushConstBlock[k].data = glm::vec4(rnd(1.0f), rnd(1.0f), rnd(1.0f), 1.0f);
			}
		}
	}
	/* 在主线程上调用 */
	void thread_Render(VkCommandBuffer primary, VkCommandBufferInheritanceInfo inherit,Frame* pFrame)
	{

		//primary.begin commandBuffer
		//primary.begin renderPass
		std::vector<VkCommandBuffer> cmds{};

		for (int t = 0; t < NUM_THREADS; t++)
		{
			for (int c = 0; c < NUM_CMD_PER_THREAD; c++)
			{
				threadPool.threads[t]->addJob(
					[=] {thread_RenderCode(t, c, inherit,pFrame);}
				);
			}
		}
		/*	等所有的thread_RenderCode 执行完毕
			之后 threadPool.thread会清空job
		*/
		threadPool.wait();
		//收集 secondary
		for (int t = 0; t < NUM_THREADS; t++)
		{
			for (int c = 0; c < NUM_CMD_PER_THREAD; c++)
			{
				cmds.push_back(threadDatas[t].cmds[c]);
			}
		}
		vkCmdExecuteCommands(primary, cmds.size(), cmds.data());
		//primary.end renderPass
		//primary.end commandBuffer
	}
	/*
		在工作线程上 记录 cmd
		RenderPass 和 FrameBuffer 通过继承得到
		cmdBI的flags是 Render_Pass_Continue
	*/
	void thread_RenderCode(uint32_t threadIdx, uint32_t cmdIdx, VkCommandBufferInheritanceInfo inherit,Frame* pFrame)
	{
		ThreadData* thread = &threadDatas[threadIdx];

		VkCommandBufferBeginInfo cmdBI{};
		cmdBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBI.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;//Render_Pass_Continue
		cmdBI.pInheritanceInfo = &inherit;

		VkCommandBuffer cmd = thread->cmds[cmdIdx];
		MG_CHECK_RESULT(vkBeginCommandBuffer(cmd, &cmdBI));
		/* 不需要 BeginRenderPass 绑定 FrameBuffer */
		mg::batches::SetViewport(cmd, { WIDTH,HEIGHT });
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, thread->pipeline);

		//在 两个贴图之间切换
		VkDescriptorSet* set = &pFrame->tex_1;
		if (cmdIdx % 2 == 1) 
		{
			set = &pFrame->tex_2;
		}
		//场景+阴影
		uint32_t dstSet = 0;
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, thread->pi_Layout, dstSet, 1, &pFrame->scene_shadow_h, 0, nullptr);
		//差异化的贴图
		dstSet = 1;
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, thread->pi_Layout, dstSet, 1, set, 0, nullptr);

		VkShaderStageFlags stageVGF = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		/* 每个 cmd 渲染 NUM_PER_CMD 个物体 */
		for (int k = 0; k < NUM_OBJECT_PER_CMD; k++) 
		{
			vkCmdPushConstants(cmd,thread->pi_Layout,stageVGF, 0,
				sizeof(ThreadPushData),
				&thread->pushConstBlock[cmdIdx * NUM_OBJECT_PER_CMD + k]);

			thread->geo->drawGeo(cmd, 1);
		}

		MG_CHECK_RESULT(vkEndCommandBuffer(cmd));
	}
	void reset_CommandBuffers()
	{
		for (int t = 0; t < NUM_THREADS; t++)
		{
			for (int c = 0; c < NUM_CMD_PER_THREAD; c++)
			{
				vkResetCommandBuffer(threadDatas[t].cmds[c], 0);
			}
		}
	}
	float rnd(float range)
	{
		std::uniform_real_distribution<float> rndDist(0.0, range);
		return rndDist(rndEngine);
	}
	
};