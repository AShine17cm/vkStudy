#pragma once
#include <vector>
#include "glm.hpp"
#include "pipelines.h"

namespace geos
{
	struct DebugPoints
	{
		struct Point
		{
			glm::mat4 mvp;
			glm::vec4 pos;
			glm::vec4 color;
		};
		mg::VulkanDevice* vulkanDevice;
		VkDevice device;

		VkDescriptorSetLayout setLayout;
		VkPipelineLayout pipeLayout;
		VkPipeline pipeline;
		uint32_t size = sizeof(DebugPoints::Point);
		std::vector<Point> points;

		void prepare(mg::VulkanDevice* vulkanDevice,VkRenderPass renderPass);
		void draw(VkCommandBuffer cmd,glm::mat4 mvp);
		void addPoint(Point point);
		void clean();
	};
}