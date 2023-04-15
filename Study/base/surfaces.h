#pragma once
#include "vulkan/vulkan.h"
#include <vector>
namespace mg 
{
namespace surfaces 
{
	/*
	¼ì²éÏÔ¿¨µÄ queue-family  extension  format-present-mode
	
	*/
	bool isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,std::vector<const char*> extensions);
	void pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, std::vector<const char*> extensions,VkPhysicalDevice* physicalDevice);
}
}
