#pragma once
#include <chrono>
#include "VulkanDevice.h"
#include "VulkanglTFModel.h"
#include "VulkanTexture.hpp"

struct PbrEnv
{
	mg::VulkanDevice* vulkanDevice;
	VkDevice device;
	VkPipelineCache pipelineCache = VK_NULL_HANDLE;

	//引用外部
	vkglTF::Model skybox;
	vks::TextureCubeMap environmentCube;
	vks::Texture2D empty;
	vks::Texture2D lutBrdf;

	//生成对象
	vks::TextureCubeMap irradianceCube;
	vks::TextureCubeMap prefilteredCube;
	//输出一个值
	float prefilteredCubeMipLevels;

	PbrEnv(mg::VulkanDevice* vulkanDevice,
		const char* emptyFile,
		const char* envFile,
		const char* skyboxModel);
	void clean();
	void generateBRDFLUT();
	void generateCubemaps();

	VkPipelineShaderStageCreateInfo loadShader(VkDevice device, std::string filename, VkShaderStageFlagBits stage);
};
