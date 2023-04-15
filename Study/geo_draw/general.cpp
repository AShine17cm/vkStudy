#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <array>
#include <optional>
#include <set>

#include "mg.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

struct UniformBufferObject {
    glm::mat4 model;//alignas(16)
    glm::mat4 view;
    glm::mat4 proj;
};
struct Frame
{
    materials::Material matA;
    materials::Material matB;
    mg::Buffer sceneUbo;
    void cleanup()
    {
        sceneUbo.destroy();
    }
};
class GeneralApp {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow* window;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;
    mg::VulkanDevice* vulkanDevice;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
    /* 渲染流程 */
    uint32_t currentFrame = 0;
    mg::SwapChain* swapchain;
    VkRenderPass renderPass;
    VkPipeline pipeline;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    std::vector<Frame> frames;

    mg::shaders::Shader shader;
    /* 渲染流程 */

    VkCommandPool commandPool;
    VkDescriptorPool descriptorPool;
    
    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    /*geos*/
    std::vector<geos::Geo*> sceneObject;
    geos::Geo* ground;
    /* resource */
    textures::Texture* tex_figure;
    textures::Texture* tex_floor;
    textures::Texture* tex_depth;

    VkFormat depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;


    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<GeneralApp*>(glfwGetWindowUserPointer(window));
    }

    void initVulkan() {
        createInstance();
        setupDebugMessenger();

        MG_CHECK_RESULT(glfwCreateWindowSurface(instance, window, nullptr, &surface));          //Surface
        mg::surfaces::pickPhysicalDevice(instance, surface, deviceExtensions, &physicalDevice); //显卡 graphics-present format-mode ex
        createLogicalDevice();
        swapchain = new mg::SwapChain(WIDTH, HEIGHT);
        swapchain->createSwapChain(physicalDevice, device, surface);
        createRenderPass();
        createGraphicsPipeline();

        /* geos */
        sceneObject.push_back(new geos::GeoCube(0.2f, { -0.5f,0.3f,0.1f }));
        sceneObject.push_back(new geos::GeoCube(0.3f, { 0.5f,0.8f,0.05f }));
        for (uint32_t i = 0; i < sceneObject.size(); i++) 
        {
            sceneObject[i]->prepareBuffer(vulkanDevice);
        }
        ground = new geos::GeoPlane(2, { 0,0,0 });
        ground->prepareBuffer(vulkanDevice);

        createResources();
        /* Framebuffers */
        swapChainFramebuffers.resize(swapchain->imageCount);
        uint32_t attachmentCount = 2;
        for (uint32_t i = 0; i < swapchain->imageCount; i++)
        {
            VkImageView images[] = { swapchain->imageViews[i],tex_depth->view };
            framebuffers::createFramebuffers(swapchain->extent, renderPass, images, attachmentCount, device, &swapChainFramebuffers[i]);
        }

        createCommandBuffers();
        createSyncObjects();
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(device);
    }

    void cleanup() {
        for (auto framebuffer : swapChainFramebuffers) 
        {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }
        swapchain->cleanup(device);
        shader.cleanup(device);
        vkDestroyRenderPass(device, renderPass, nullptr);
        vkDestroyPipeline(device, pipeline, nullptr);
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
        {
            frames[i].cleanup();
        }
        tex_figure->destroy();
        tex_floor->destroy();
        tex_depth->destroy();

        vkDestroyDescriptorPool(device, descriptorPool, nullptr);

        for (uint32_t i = 0; i < sceneObject.size(); i++) 
        {
            sceneObject[i]->clean();
        }
        ground->clean();

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(device, commandPool, nullptr);
        vkDestroyDevice(device, nullptr);

        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void createInstance() 
    {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Mg App";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Mg Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        /*
        surface 扩展
        */
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);//surface win32_surface
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        createInfo.enabledExtensionCount = extensions.size();
        createInfo.ppEnabledExtensionNames = extensions.data();
        //createInfo.enabledLayerCount = 0;           //VK_LAYER_KHRONOS_validation  vkEnumerateInstanceLayerProperties( );
        //createInfo.pNext = nullptr;

        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

        MG_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &instance));
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    void setupDebugMessenger() 
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);
        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    void createLogicalDevice() 
    {
        VkPhysicalDeviceFeatures deviceFeatures{};
        vulkanDevice = new mg::VulkanDevice(physicalDevice);
        vulkanDevice->createLogicalDevice(deviceFeatures, deviceExtensions, nullptr);//queueCI, command-pool
        //
        device = vulkanDevice->logicalDevice;
        commandPool = vulkanDevice->commandPool;
        graphicsQueue = vulkanDevice->graphicsQueue;
        presentQueue = vulkanDevice->presentQueue;
    }

    void createRenderPass() 
    {
        mg::renderpasses::createRenderPass(swapchain->imageFormat,depthFormat, device, &renderPass);
    }

    void createGraphicsPipeline() {

        std::vector<VkVertexInputAttributeDescription> attributes;
        std::vector<VkVertexInputBindingDescription> bindings;
        VkPipelineVertexInputStateCreateInfo vertexInputInfo =geos::GeoCube::getVertexInput(&attributes, &bindings);

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        //顺序绑定
        shader.typeSet = {  VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER  };
        shader.stageSet = {  VK_SHADER_STAGE_VERTEX_BIT,VK_SHADER_STAGE_FRAGMENT_BIT  };
        shader.descCountSet = { 1,1 };
        shader.pushRanges = { { VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(mat4) } };
        shader.createDescLayout(device);

        shader.shaderFiles = { "shaders/vert.spv", "shaders/frag.spv" };
        shader.shaderStageFlags={ VK_SHADER_STAGE_VERTEX_BIT,VK_SHADER_STAGE_FRAGMENT_BIT };
        shader.createPipeline(&colorBlendAttachment,1, &vertexInputInfo, renderPass, device,&pipeline);
    }

    void createResources() 
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);
        frames.resize(MAX_FRAMES_IN_FLIGHT);
        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
        {
            vulkanDevice->createBuffer(
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                bufferSize, &frames[i].sceneUbo);
            frames[i].sceneUbo.map();//给出mapped地址
        }
        /* Textures */
        textures::MgImageInfo extent = { {0,0,1},1,1 };
        extent.formats = {
            VK_IMAGE_TYPE_2D,
            VK_FORMAT_R8G8B8A8_SRGB ,
            VK_IMAGE_USAGE_SAMPLED_BIT ,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL  };
        tex_figure = new textures::Texture(vulkanDevice,extent);
        tex_figure->load("../textures/figure01.jpg");
        tex_floor = new textures::Texture(vulkanDevice, extent);
        tex_floor->load("../textures/checker.jpg");
        /* Depth */
        extent = { {swapchain->extent.width,swapchain->extent.height, 1},1,1 };
        extent.formats = {
            VK_IMAGE_TYPE_2D,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
        };
        tex_depth = new textures::Texture(vulkanDevice, extent);
        tex_depth->load(nullptr);

        /* Pool */
        std::vector<VkDescriptorPoolSize> poolSizes =
        {
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,MAX_FRAMES_IN_FLIGHT + 8},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,MAX_FRAMES_IN_FLIGHT + 8}
        };
        mg::descriptors::createDescriptorPool(poolSizes.data(), poolSizes.size(), device, &descriptorPool, nullptr, MAX_FRAMES_IN_FLIGHT + 8);

        /* Materials */
        mg::materials::Material* pMat;
        mg::shaders::Shader* pShader;

        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            pMat= &frames[i].matA;
            pShader = &shader;
            pMat->pShader = &shader;
            pMat->writeInfoSet = { &frames[i].sceneUbo.descriptor,&tex_figure->descriptor };
            //分配-写 DescriptorSet
            pMat->writeDescriptorSets(device, descriptorPool);

            pMat = &frames[i].matB;
            pShader = &shader;
            pMat->pShader = &shader;
            pMat->writeInfoSet = { &frames[i].sceneUbo.descriptor,&tex_floor->descriptor };
            pMat->writeDescriptorSets(device, descriptorPool);
        }
    }

    void createCommandBuffers() 
    {
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        vulkanDevice->createCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY,commandBuffers.data() , MAX_FRAMES_IN_FLIGHT);
    }

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) 
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        MG_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));

            std::array<VkClearValue, 2> clears{};
            clears[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
            clears[1].depthStencil = { 1.0f,0 };
            //VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f} } };
            mg::batches::BeginRenderpass(commandBuffer, renderPass, swapChainFramebuffers[imageIndex],clears.data(), 2, swapchain->extent);
            mg::batches::SetViewport(commandBuffer, swapchain->extent);
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
            uint32_t size = sizeof(mat4);
            frames[currentFrame].matA.beginMaterial(commandBuffer);
            for (uint32_t i = 0; i < sceneObject.size(); i++) 
            {
                vkCmdPushConstants(commandBuffer, frames[currentFrame].matA.pShader->layout, VK_SHADER_STAGE_VERTEX_BIT, 0,size, &sceneObject[i]->modelMatrix);
                sceneObject[i]->drawGeo(commandBuffer);
            }
            //最后画ground 故意的
            frames[currentFrame].matB.beginMaterial(commandBuffer);
            vkCmdPushConstants(commandBuffer, frames[currentFrame].matB.pShader->layout, VK_SHADER_STAGE_VERTEX_BIT, 0, size, &ground->modelMatrix);
            ground->drawGeo(commandBuffer);

        vkCmdEndRenderPass(commandBuffer);

        MG_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));
    }

    void createSyncObjects() {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    void updateUniformBuffer(uint32_t currentFrame) {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        //time = 0;
        UniformBufferObject ubo{};
        //ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), swapchain->extent.width / (float) swapchain->extent.height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;


        VkDeviceSize size = sizeof(UniformBufferObject);
        memcpy(frames[currentFrame].sceneUbo.mapped, &ubo, size);

        //模型的位置
        for (uint32_t i = 0; i < sceneObject.size(); i++) 
        {
            mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, sceneObject[i]->pos);
            model= glm::rotate(model, time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            sceneObject[i]->modelMatrix = model;
        }
        ground->modelMatrix = glm::mat4(1.0f);
    }

    void drawFrame() {
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, swapchain->swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }
        if (result == VK_ERROR_OUT_OF_DATE_KHR) 
        {
            return;
        } 

        updateUniformBuffer(currentFrame);

        vkResetFences(device, 1, &inFlightFences[currentFrame]);

        vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
        recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = {swapchain->swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        result = vkQueuePresentKHR(presentQueue, &presentInfo);

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }
};

int main() {
    GeneralApp app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
