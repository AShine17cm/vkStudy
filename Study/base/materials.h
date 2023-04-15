#pragma once
#include "vulkan/vulkan.h"
#include <vector>
#include "shaders.h"

namespace mg 
{
namespace materials 
{
	/*
	材质级别
	*/
	struct Material
	{
		mg::shaders::Shader* pShader;
		VkDescriptorSet descriptorSet;			//vert:<ubo,tex>	frag:<ubo,tex>
		std::vector<void*> writeInfoSet;		//vert:<ubo,tex>	frag:<ubo,tex>
		//分配 并 写 DescriptorSet
		void writeDescriptorSets(VkDevice device,VkDescriptorPool descriptorPool);

		void beginMaterial(VkCommandBuffer cmd);
	};
}
}

