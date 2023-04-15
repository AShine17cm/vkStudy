#pragma once
#include "vulkan/vulkan.h"
#include <random>
#include <array>
#include <vector>
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "Geo.h"
#include "VulkanDevice.h"
#include "VulkanTools.h"
#include "descriptors.h"

using namespace mg;
/*
ʹ�� DescriptorSet ���� Instance����
UBO ���1024*64,ʵ������������Դ��ڻ���С��  shader�е�����???
*/

struct InstanceData
{
	glm::mat4 model;
	glm::vec4 texIndex;	//alignas int ��shader��ȡ������ȷ��ֵ
};

struct InstanceByDescriptorSet
{
	static const uint32_t instanceCount = 1024 * 64 / (8 * (16 + 4));
	float radius = 4.0f;		//�ֲ��뾶
	float height = 1.0f;		//Բ���ĸ߶�
	geos::Geo* geo;

	/* ʹ�� descriptorSet,û�з�֡ */
	std::array<InstanceData, instanceCount> instances;
	std::array<vec3,instanceCount> tangents;			//ÿһ�����������ָ��
	uint32_t instanceCount;
	mg::Buffer instanceBuffer;
	VkDescriptorSet set_Instances;

	void prepare(VulkanDevice* vulkanDevice, geos::Geo* item, VkDescriptorPool descriptorPool,VkDescriptorSetLayout* layout, uint32_t texLayerCount)
	{
		this->geo = item;
		this->instances = instances;

		std::default_random_engine rnd(0);
		std::uniform_real_distribution<float> uniformDist(0, 0.4f);
		std::uniform_int_distribution<uint32_t> rndTextureIndex(0, texLayerCount);
		std::uniform_real_distribution<float> scales(0.9f, 1.1f);
		float r, theta;
		float scale;
		float stepAng = glm::pi<float>() * 2.0f / instanceCount;
		float ang = 0;
		for (uint32_t i = 0; i < instanceCount; i++)
		{
			/* �뾶-����-��С */
			r = radius + sqrt(uniformDist(rnd) - 0.2f);
			ang = stepAng * (i + uniformDist(rnd) * 0.2f);
			scale = scales(rnd);

			/* ���� */
			glm::mat4 model = glm::mat4(1.0f);
			/* λ�� ��ת ��С */
			float sin = glm::sin(theta);
			float cos = glm::cos(theta);
			model = glm::translate(model, glm::vec3(r * sin, r * cos, height));
			model = glm::scale(model, { scale,scale,scale });
			model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			model = glm::rotate(model, glm::radians(60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			/* ��ͼ */
			uint32_t texIndex = rndTextureIndex(rnd) % texLayerCount;
			instances[i] = { model,glm::vec4(texIndex) };
			/* �з��� */
			tangents[i] = { cos,sin,0.0f };
		}

		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			sizeof(InstanceData) * instanceCount,
			&instanceBuffer);
		instanceBuffer.map();

		memcpy(instanceBuffer.mapped, instances.data(), sizeof(InstanceData) * instanceCount);

		//����descriptor-set
		descriptors::allocateDescriptorSet(layout, 1, descriptorPool, vulkanDevice->logicalDevice, &set_Instances);
		std::vector<VkDescriptorType> types = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };
		std::vector<uint32_t> counts = { 1 };
		std::vector<void*> infos = { &instanceBuffer.descriptor };
		mg::descriptors::writeDescriptorSet(types.data(), infos.data(), counts.data(), counts.size(), set_Instances, vulkanDevice->logicalDevice);
	}
	void update(float deltaTime)
	{
		/* ��ת�������� */
		float ang = glm::radians(60.0f) * deltaTime;
		for (uint32_t i = 0; i < instanceCount; i++) 
		{
			mat4 model = instances[i].model;
			instances[i].model = glm::rotate(model, ang, tangents[i]);
		}
		memcpy(instanceBuffer.mapped, instances.data(), sizeof(InstanceData) * instanceCount);
	}
	void draw(VkCommandBuffer cmd)
	{
		uint32_t indexCount = geo->triangles.size();
		uint32_t instanceCount = this->instanceCount;
		uint32_t firstIndex = 0;
		uint32_t vertexOffset = 0;
		uint32_t firstInstance = 0;
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(cmd, 0, 1, &geo->vertexBuffer.buffer, offsets);
		vkCmdBindIndexBuffer(cmd, geo->indexBuffer.buffer, 0, geo->indexType);
		vkCmdDrawIndexed(cmd, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}
	void cleanup()
	{
		geo->clean();
		instanceBuffer.destroy();
	}
};
