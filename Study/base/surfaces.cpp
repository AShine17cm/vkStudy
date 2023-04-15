#include "surfaces.h"
#include <vector>
#include <optional>
#include <set>
#include <string>
/*
���� surface-swapchain ��صĴ���
*/
namespace mg 
{
namespace surfaces 
{
    //����Կ��� queue-family  extension  format-present-mode
    bool isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,std::vector<const char*> extensions) 
    {
        //queue
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
        //format+present
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
        //extensions
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());//������һ���µļ�����
        for (const auto& extension : availableExtensions) 
        {
            requiredExtensions.erase(extension.extensionName);//���ɾ��
        }

        int i = 0;
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                graphicsFamily = i;
            }
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
            if (presentSupport)
            {
                presentFamily = i;
            }
            if (graphicsFamily.has_value() && presentFamily.has_value())
            {
                break;
            }
            i++;
        }

        return
            graphicsFamily.has_value() && presentFamily.has_value() &&
            formatCount * presentModeCount > 0 &&
            requiredExtensions.empty();

    }
	void pickPhysicalDevice(VkInstance instance,VkSurfaceKHR surface,std::vector<const char*> extensions, VkPhysicalDevice* physicalDevice)
	{
        //�Կ� graphics+present(present һ��������queue family�غ�)
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto& device : devices)
        {
            if (isDeviceSuitable(device,surface,extensions)) 
            {
                *physicalDevice = device;
                break;
            }
        }
	}
    
}
}