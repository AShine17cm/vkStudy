#pragma once
#include "vulkan/vulkan.h"

struct QueryPool
{
	VkDevice device;
	VkQueryPool pool;						//occlusion ²éÑ¯
	uint64_t passedSamples[2] = { 1,1 };	//Passed query samples

	QueryPool(VkDevice device) 
	{
		this->device = device;
	}
	void prepare() 
	{
		VkQueryPoolCreateInfo queryPoolCI = {};
		queryPoolCI.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
		queryPoolCI.queryType = VK_QUERY_TYPE_OCCLUSION;
		queryPoolCI.queryCount = 2;
		MG_CHECK_RESULT(vkCreateQueryPool(device, &queryPoolCI, nullptr, &pool));
	}
	void getQueryResults() 
	{
		uint32_t firstQuery = 0;
		uint32_t queryCount = 2;
		VkDeviceSize stride = sizeof(uint64_t);
		VkQueryResultFlags queryFlags = VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT;
		// Store results a 64 bit values and wait until the results have been finished
		// If you don't want to wait, you can use VK_QUERY_RESULT_WITH_AVAILABILITY_BIT
		// which also returns the state of the result (ready) in the result
		vkGetQueryPoolResults(
			device, pool, 
			firstQuery, queryCount, 
			sizeof(passedSamples), passedSamples,
			stride, queryFlags);
	}
	void cleanup() 
	{
		vkDestroyQueryPool(device, pool, nullptr);
	}
};
