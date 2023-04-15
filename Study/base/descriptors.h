#pragma once
#include <vector>
#include "vulkan/vulkan.h"
#include "VulkanTools.h"
namespace mg
{
	namespace descriptors
	{
		void createDescriptorSetLayout(
			VkDescriptorType* types,
			VkShaderStageFlags* stages,
			uint32_t bindCount,
			VkDevice device,
			VkDescriptorSetLayout* layout,
			uint32_t descriptorCount = 1,
			uint32_t extend = 0,
			void* pNext = nullptr);
		//->
		void createDescriptorSetLayout(
			VkDescriptorType* types,
			VkShaderStageFlags* stages,
			uint32_t* descriptorCounts,
			uint32_t bindCount,
			VkDevice device,
			VkDescriptorSetLayout* layout,
			//uint32_t offsetBinding,
			uint32_t extend = 0,
			void* pNext = nullptr);

		void allocateDescriptorSet(
			VkDescriptorSetLayout* layouts,
			uint32_t layoutCount,
			VkDescriptorPool pool,
			VkDevice device,
			VkDescriptorSet* descriptorSet,
			void* pNext = nullptr);
		//
		void writeDescriptorSet(
			VkDescriptorType* types,
			void** descriptorInfos,
			uint32_t* descriptorCounts,
			uint32_t typeCount,
			VkDescriptorSet descriptorSet,
			VkDevice device,
			//uint32_t offsetBinding,
			uint32_t inlineStride = 0);

		void writeDescriptorSet(
			VkDescriptorType* types,
			void** descriptorInfos,
			uint32_t typeCount,
			VkDescriptorSet descriptorSet,
			VkDevice device,
			uint32_t inlineStride = 0);

		void createDescriptorPool(
			VkDescriptorPoolSize* poolSizes,
			uint32_t poolSizCount,
			VkDevice device,
			VkDescriptorPool* pool,
			void* pNext = nullptr,
			uint32_t maxSets = 2);

	}
}
