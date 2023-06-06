#pragma once
#include "glm.hpp"
#include <vulkan/vulkan.h>

namespace geos 
{
	/* �� PushConstant������ */
	struct PerObjectData
	{
		glm::mat4 model;
		glm::vec4 texIndex;
	};
	struct PbrBasic
	{
		glm::vec4 rgba;
		float roughness;
		float metallic;

		//�߼�����
		float exposure;
		float gamma;
		float prefilteredCubeMipLevels;
		float scaleIBLAmbient;
		float debugViewInputs = 0;
		float debugViewEquation = 0;
	};
	struct PbrMaterial
	{
		glm::vec4 baseColorFactor = { 1,1,1,1 };
		glm::vec4 emissiveFactor = { 1,1,1,1 };
		glm::vec4 diffuseFactor = { 1,1,1,1 };
		glm::vec4 specularFactor = { 0,0,0,1 };

		int occlusionTextureSet = 0;
		int emissiveTextureSet = -1;

		float metallicFactor = 1.0f;
		float roughnessFactor = 1.0f;
		float alphaMask = 0.0f;
		float alphaMaskCutoff = 1.0f;

		//�߼�����
		float exposure = 4.5f;
		float gamma = 2.2f;
		float prefilteredCubeMipLevels;	//��Ҫ����
		float scaleIBLAmbient = 1.0f;
		float debugViewInputs = 0.0f;
		float debugViewEquation = 0.0f;

		PbrMaterial();
	};
	//������Ⱦһ�� gltf��������Ϣ �߹���
	struct gltfPbrRender_spec 
	{
		bool isMetallic = false;
		VkDescriptorImageInfo* emptyImg;
		VkDescriptorImageInfo* colorImg;
		VkDescriptorImageInfo* normalImg;
		VkDescriptorImageInfo* ocImg;
		VkDescriptorImageInfo* specImg;

		VkDescriptorImageInfo* metalRough;	//������
		VkDescriptorImageInfo* emissiveImg;

		//VkDescriptorSet toRoot;			//�����ڵ�ľ���
		//geos::PerObjectData pod;
		glm::mat4 model;

		PbrMaterial mat;
	};
}