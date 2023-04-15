#pragma once
#include <vulkan/vulkan.h>
#include <iostream>
#include <assert.h>

// Custom define for better code readability
#define VK_FLAGS_NONE 0
// Default fence timeout in nanoseconds
#define DEFAULT_FENCE_TIMEOUT 100000000000

#define MG_CHECK_RESULT(f)						\
{												\
	VkResult result = (f);						\
	if (VK_SUCCESS != result)					\
	{											\
	std::cerr << "Fatal error:" << result << "in file:" << __FILE__ << " at line:" << __LINE__ << "\n";	\
	assert(VK_SUCCESS == result);				\
	}											\
}																								

namespace mg
{
	/*
	namespace tools
	{
		// Display error message and exit on fatal error
		void exitFatal(const std::string& message, int32_t exitCode);
		void exitFatal(const std::string& message, VkResult resultCode);
		bool fileExists(const std::string& filename);
	}
	*/
}


