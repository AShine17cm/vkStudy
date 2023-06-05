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
		glm::vec4 baseColorFactor;
		glm::vec4 emissiveFactor;
		glm::vec4 diffuseFactor;
		glm::vec4 specularFactor;

		int baseColorTextureSet;
		int physicalDescriptorTextureSet;
		int normalTextureSet;
		int occlusionTextureSet;
		int emissiveTextureSet;

		float metallicFactor;
		float roughnessFactor;
		float alphaMask;
		float alphaMaskCutoff;

		//�߼�����
		float exposure;
		float gamma;
		float prefilteredCubeMipLevels;
		float scaleIBLAmbient;
		float debugViewInputs;
		float debugViewEquation;

		PbrMaterial();
	};
	//������Ⱦһ�� gltf��������Ϣ �߹���
	struct gltfPbrRender_spec 
	{
		VkDescriptorImageInfo* colorImg;
		VkDescriptorImageInfo* normalImg;
		VkDescriptorImageInfo* ocImg;
		VkDescriptorImageInfo* specImg;

		PbrMaterial mat;
	};
}