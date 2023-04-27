#pragma once
#include "vulkan/vulkan.h"
#include <array>
#include <random>
#include <vector>
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "Geo.h"
#include "VulkanDevice.h"
#include "VulkanTools.h"
#include "descriptors.h"
using namespace mg;
/* 
定制管线，使用 pipeline-vertex-input 绑定第二套 RATE_INSTANCE数据
渲染阶段，使用 vkCmdBindVertexBuffers 绑定Instance 数据
*/
struct InstanceData
{
	uint32_t texIndex;
	glm::vec3 color;
	glm::mat4 model;
};
struct InstanceByVertexBuffers
{
	static const uint32_t instanceCount = 128;
	const float radius=1.5;
	geos::Geo* geo;

	/* 直接cmd bind vertex-input,不需要descriptorSet */
	std::array<InstanceData, instanceCount> instances;
	mg::Buffer instanceBuffer;

	void prepare(VulkanDevice* vulkanDevice, uint32_t texLayerCount) 
	{
		vec3 clipA = { 0.5f,0.3f,0.2f };
		vec3 clipB = { 0.4f,0.2f,0.45f };
		geo = new geos::GeoCube(0.1f,clipA,clipB);
		geo->prepareBuffer(vulkanDevice);
		geo->name = "Instancing Item";

		std::default_random_engine rndGenerator(0);
		std::uniform_real_distribution<float> uniformDist(0.0, 1.0);
		std::uniform_int_distribution<uint32_t> rndTextureIndex(0,texLayerCount);
		std::uniform_real_distribution<float> scales(1.0f, 1.3f);
		float r, theta;
		float scale;
		for (uint32_t i = 0; i < instanceCount; i++) 
		{
			r = sqrt(uniformDist(rndGenerator))*radius;
			theta = 2.0 * 3.14 * uniformDist(rndGenerator);
			scale = scales(rndGenerator);
			glm::mat4 model = glm::mat4(1.0f);
			//随机位置 旋转 大小
			model = glm::translate(model, glm::vec3(r*glm::sin(theta),r*glm::cos(theta),0));
			model = glm::scale(model, { scale,scale,scale });
			model = glm::rotate(model, theta, glm::vec3(scale, 0.0f, 1/scale));
			//随机贴图 颜色
			instances[i] = {  rndTextureIndex(rndGenerator)%texLayerCount,{r / radius,r / radius,0.5f},model };
		}
		MG_CHECK_RESULT(vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			sizeof(InstanceData) * instanceCount,
			&instanceBuffer,
			instances.data()));

		
		instanceBuffer.map();
		memcpy(instanceBuffer.mapped, instances.data(), sizeof(InstanceData) * instanceCount);

	}
	void update(float time) 
	{

	}
	void draw(VkCommandBuffer cmd) 
	{
		uint32_t indexCount = geo->triangles.size();
		uint32_t instanceCount =this->instanceCount;
		uint32_t firstIndex = 0;
		uint32_t vertexOffset = 0;
		uint32_t firstInstance = 0;
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(cmd, 0, 1, &geo->vertexBuffer.buffer, offsets);
		vkCmdBindVertexBuffers(cmd, 1, 1, &instanceBuffer.buffer, offsets);
		vkCmdBindIndexBuffer(cmd, geo->indexBuffer.buffer, 0, geo->indexType);
		vkCmdDrawIndexed(cmd, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}
	void cleanup() {
		geo->clean();
		instanceBuffer.destroy();
	}
};