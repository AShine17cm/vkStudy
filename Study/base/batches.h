#pragma once
#include  <vector>
#include "vulkan/vulkan.h"
#include "Geo.h"
#include "pipelines.h"
#include "shaders.h"
namespace mg 
{
namespace batches 
{
	void BeginRenderpass(
		VkCommandBuffer cmd,
		VkRenderPass renderPass,
		VkFramebuffer framebuffer,
		VkClearValue* clearValues,
		uint32_t clearCount,
		VkExtent2D extent
	);
	void SetViewport(
		VkCommandBuffer cmd,
		VkExtent2D extent);


}
}
