#pragma once
#include "vulkan/vulkan.h"
#include <vector>
#include "shaders.h"

namespace mg 
{
namespace materials 
{
	/*
	���ʼ���
	*/
	struct Material
	{
		mg::shaders::Shader* pShader;
		VkDescriptorSet descriptorSet;			//vert:<ubo,tex>	frag:<ubo,tex>
		std::vector<void*> writeInfoSet;		//vert:<ubo,tex>	frag:<ubo,tex>
		//���� �� д DescriptorSet
		void writeDescriptorSets(VkDevice device,VkDescriptorPool descriptorPool);

		void beginMaterial(VkCommandBuffer cmd);
	};
}
}

