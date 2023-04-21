
#include "descriptors.h"

namespace mg
{
	namespace descriptors
	{
		//顺序 添加的 binding
		void createDescriptorSetLayout(
			VkDescriptorType* types,
			VkShaderStageFlags* stages,
			uint32_t* descriptorCounts,
			uint32_t bindCount,
			VkDevice device,
			VkDescriptorSetLayout* layout,
			uint32_t extend,
			void* pNext)
		{
			std::vector<VkDescriptorSetLayoutBinding> bindings;
			for (uint32_t d = 0; d < bindCount; d++)
			{
				VkDescriptorSetLayoutBinding bind = {};
				bind.descriptorType = types[d];
				bind.stageFlags = stages[d];
				bind.binding = d;
				bind.descriptorCount = descriptorCounts[d];
				bindings.push_back(bind);
			}

			VkDescriptorSetLayoutCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			createInfo.bindingCount = bindings.size();
			createInfo.pBindings = bindings.data();
			createInfo.pNext = nullptr;
			//Setting this flag tells the descriptor set layouts that no actual descriptor sets are allocated 
			//but instead pushed at command buffer creation time
			if (extend == 1)
			{
				createInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
			}

			MG_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &createInfo, nullptr, layout));
		}
		//顺序 添加的 binding
		void createDescriptorSetLayout(
			VkDescriptorType* types,
			VkShaderStageFlags* stages,
			uint32_t bindCount,
			VkDevice device,
			VkDescriptorSetLayout* layout,
			uint32_t descriptorCount,
			uint32_t extend,
			void* pNext)
		{
			std::vector<uint32_t> descriptorCounts;

			for (uint32_t d = 0; d < bindCount; d++)
			{
				descriptorCounts.push_back(descriptorCount);
			}

			createDescriptorSetLayout(types, stages, descriptorCounts.data(), bindCount, device, layout, extend, pNext);
		}

		void allocateDescriptorSet(
			VkDescriptorSetLayout* layouts,
			uint32_t layoutCount,
			VkDescriptorPool pool,
			VkDevice device,
			VkDescriptorSet* descriptorSet,
			void* pNext)
		{
			VkDescriptorSetAllocateInfo allocateInfo{};
			allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocateInfo.descriptorPool = pool;
			allocateInfo.pSetLayouts = layouts;
			allocateInfo.descriptorSetCount = layoutCount;
			allocateInfo.pNext = pNext;
			//VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT or VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT 
			
			MG_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocateInfo, descriptorSet));
		}

		void writeDescriptorSet(
			VkDescriptorType* types,
			void** descriptorInfos,
			uint32_t* descriptorCounts,
			uint32_t typeCount,
			VkDescriptorSet descriptorSet,
			VkDevice device,
			uint32_t inlineStride)
		{
			std::vector<VkWriteDescriptorSet> writes;
			for (uint32_t i = 0; i < typeCount; i++) {
				VkWriteDescriptorSet write{};
				write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write.dstSet = descriptorSet;
				write.descriptorType = types[i];
				write.descriptorCount = descriptorCounts[i];
				write.dstBinding = i;
				write.dstArrayElement = 0;

				switch (types[i])
				{
				case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
				case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
					write.pBufferInfo = (VkDescriptorBufferInfo*)descriptorInfos[i];
					break;
				case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
				case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:	//deferred, display depth
				case VK_DESCRIPTOR_TYPE_SAMPLER:			//texture mipmap gen
				case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:		//texture mipmap gen
					write.pImageInfo = (VkDescriptorImageInfo*)descriptorInfos[i];
					break;
				case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT:
					write.pNext = descriptorInfos[i];
					write.descriptorCount = inlineStride;
					break;
				default:
					break;
				}

				writes.push_back(write);
			}

			vkUpdateDescriptorSets(device, writes.size(), writes.data(), 0, nullptr);
		}

		void writeDescriptorSet(
			VkDescriptorType* types,
			void** descriptorInfos,
			uint32_t typeCount,
			VkDescriptorSet descriptorSet,
			VkDevice device,
			uint32_t inlineStride)
		{
			std::vector<uint32_t> descriptorCounts;
			for (uint32_t i = 0; i < typeCount; i++) {
				descriptorCounts.push_back(1);
			}
			writeDescriptorSet(types, descriptorInfos, descriptorCounts.data(), typeCount, descriptorSet, device, inlineStride);
		}

		void createDescriptorPool(
			VkDescriptorPoolSize* poolSizes,
			uint32_t poolSizCount,
			VkDevice device,
			VkDescriptorPool* pool,
			void* pNext,
			uint32_t maxSets)
		{
			VkDescriptorPoolCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			createInfo.maxSets = maxSets;
			createInfo.pPoolSizes = poolSizes;
			createInfo.poolSizeCount = poolSizCount;
			createInfo.pNext = pNext;

			MG_CHECK_RESULT(vkCreateDescriptorPool(device, &createInfo, nullptr, pool));
		}

	}
}