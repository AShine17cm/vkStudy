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
#include "Frame.h"
using namespace mg;
/* 
使用 单独的一个 DescriptorSet 输入 Instance数据
UBO 最大1024*64
实际填充数量可以大于或者小于  shader中的数组???
*/

struct InstanceData
{
	glm::mat4 model;
	glm::vec4 texIndex;	//alignas int 在shader中取不到正确的值
};
struct InstanceByDescriptorSet
{
	static const uint32_t maxCount = 1024*64/(8*20);
	static const uint32_t instanceCount =1024*64/(8*20);
	const float radius=1.5f;
	geos::Geo* geo;

	/* 使用 descriptorSet,没有分帧 */
	std::array<InstanceData, instanceCount> instances;
	mg::Buffer instanceBuffer;
	VkDescriptorSet set_Instances;

	void prepare(VulkanDevice* vulkanDevice, VkDescriptorPool descriptorPool, PipelineHub* pipes,uint32_t texLayerCount)
	{
		/* 立方体  将被Instancing的物体 */
		float size = 0.1f;
		vec3 clipA = { 0.5f,0.3f,0.2f };
		vec3 clipB = { 0.4f,0.2f,0.45f };
		geo = new geos::GeoCube(size, clipA, clipB);

		geo->prepareBuffer(vulkanDevice);
		geo->name = "Instancing Item";

		std::default_random_engine rndGenerator(0);
		std::uniform_real_distribution<float> uniformDist(0, 1.0f);
		std::uniform_int_distribution<uint32_t> rndTextureIndex(0,texLayerCount);
		std::uniform_real_distribution<float> scales(0.3f, 1.0f);
		float r, theta;
		float scale;
		for (uint32_t i = 0; i < instanceCount; i++) 
		{
			r = sqrt(uniformDist(rndGenerator)+0.5f)*radius;
			theta =2.0 * 3.14 * uniformDist(rndGenerator);
			scale = scales(rndGenerator);
			glm::mat4 model = glm::mat4(1.0f);
			//随机位置 旋转 大小
			model = glm::translate(model, glm::vec3(r*glm::sin(theta),r*glm::cos(theta),0));
			model = glm::scale(model, { scale,scale,scale });
			model = glm::rotate(model, theta, glm::vec3(scale, 0.0f, 1.0f));
			//随机贴图 颜色
			uint32_t texIndex = rndTextureIndex(rndGenerator) % texLayerCount;
			instances[i] = { model,glm::vec4(texIndex) };
		}

		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			sizeof(InstanceData) * instanceCount,
			&instanceBuffer);
		instanceBuffer.map();

		memcpy(instanceBuffer.mapped, instances.data(), sizeof(InstanceData) * instanceCount);

		//操作descriptor-set
		descriptors::allocateDescriptorSet(&pipes->setLayout_InstanceData, 1, descriptorPool, vulkanDevice->logicalDevice, &set_Instances);
		std::vector<VkDescriptorType> types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };
		std::vector<uint32_t> counts = { 1 };
		std::vector<void*> infos = {&instanceBuffer.descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), set_Instances, vulkanDevice->logicalDevice);
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
		vkCmdBindIndexBuffer(cmd, geo->indexBuffer.buffer, 0, geo->indexType);
		vkCmdDrawIndexed(cmd, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}
	void cleanup() {
		geo->clean();
		instanceBuffer.destroy();
	}
};