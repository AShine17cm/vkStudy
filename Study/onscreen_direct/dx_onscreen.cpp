#include "misc.h"
#include "mg.h"

#include "PipelineHub.h"
#include "Frame.h"
#include "Scene.h"

/*
无 vertex buffer, 通过vertex.shader 直接在屏幕上画
*/
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
    std::vector<VkFramebuffer> swapChainFramebuffers;
    std::vector<Frame> frames;

    PipelineHub *piHub;

    /* 渲染流程 */

    VkCommandPool commandPool;
    VkDescriptorPool descriptorPool;
    /*geos*/
    Scene scene;
    /* resource */
    Resource resource;

    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    VkFormat depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;
    /* 控制台 清空*/
    float time = 0.0f;

    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        glfwSetWindowSizeLimits(window, WIDTH, HEIGHT, WIDTH, HEIGHT);
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<GeneralApp*>(glfwGetWindowUserPointer(window));
    }

    void initVulkan() {
        createInstance();
        setupDebugMessenger(instance,&debugMessenger);

        MG_CHECK_RESULT(glfwCreateWindowSurface(instance, window, nullptr, &surface));          //Surface
        mg::surfaces::pickPhysicalDevice(instance, surface, deviceExtensions, &physicalDevice); //显卡 graphics-present format-mode ex
        createLogicalDevice();
        swapchain = new mg::SwapChain(WIDTH, HEIGHT);
        swapchain->createSwapChain(physicalDevice, device, surface);
        createRenderPass();

        createGraphicsPipeline();
        /* geos */
        scene.prepare(vulkanDevice);

        createResources();
        /* Framebuffers */
        swapChainFramebuffers.resize(swapchain->imageCount);
        uint32_t attachmentCount = 2;
        for (uint32_t i = 0; i < swapchain->imageCount; i++)
        {
            VkImageView images[] = { swapchain->imageViews[i],resource.tex_depth->view };
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
        vkDestroyRenderPass(device, renderPass, nullptr);
        piHub->cleanup(device);
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
        {
            frames[i].cleanup(device);
        }
        resource.cleanup();

        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        scene.cleanup();

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
        uint32_t pushSize = sizeof(glm::vec4);
        piHub = new PipelineHub();
        piHub->prepare(device, renderPass,pushSize);
    }

    void createResources() 
    {
        uint32_t countUBO = MAX_FRAMES_IN_FLIGHT * ((1 + 4 + 1) + (1 + 1));
        uint32_t countSampler = MAX_FRAMES_IN_FLIGHT * (1 + 1+1);
        /* Pool */
        std::vector<VkDescriptorPoolSize> poolSizes =
        {
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,countUBO},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,countSampler}
        };
        uint32_t maxSets = MAX_FRAMES_IN_FLIGHT * ((1 + 1) + (1 + 1));
        mg::descriptors::createDescriptorPool(poolSizes.data(), poolSizes.size(), device, &descriptorPool, nullptr, maxSets);
        /* Tex 和 depth-Tex */
        resource.prepare(vulkanDevice,swapchain->extent);

        /* Frame */
        frames.resize(MAX_FRAMES_IN_FLIGHT);
        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
        {
            frames[i].prepare(vulkanDevice, descriptorPool, piHub);
            frames[i].updateDescritors(device,&resource);
        }
    }

    void createCommandBuffers() 
    {
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        vulkanDevice->createCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY,commandBuffers.data() , MAX_FRAMES_IN_FLIGHT);
    }
    /* 帧级别的渲染 */
    void recordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex) 
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        MG_CHECK_RESULT(vkBeginCommandBuffer(cmd, &beginInfo));

        std::array<VkClearValue, 2> clears{};
        clears[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clears[1].depthStencil = { 1.0f,0 };

        Frame* frame = &frames[currentFrame];
        uint32_t dstSet = 0;
        /* Offscreen */
        mg::batches::BeginRenderpass(cmd,renderPass, swapChainFramebuffers[imageIndex], clears.data(), 2, swapchain->extent);
        mg::batches::SetViewport(cmd, swapchain->extent);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub->piLayout_screen, dstSet, 1, &frame->setScreen, 0, nullptr);
        
        drawScene(cmd, frame);     

        vkCmdEndRenderPass(cmd);
        MG_CHECK_RESULT(vkEndCommandBuffer(cmd));
    }
    /* 使用一个 scene相关的set  画一遍场景 */
    void drawScene(VkCommandBuffer cmd, Frame* frame) 
    {
        uint32_t dstSet = 0;
        uint32_t batchIdx = 0;
        /* pipeline-Solid */
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub->pi_screen);
        scene.draw(cmd, piHub->piLayout_screen, batchIdx);

        batchIdx = 1;
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub->pi_scrArray);
        scene.draw(cmd, piHub->piLayout_screen, batchIdx);

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

        VkDeviceSize size = sizeof(glm::vec4);
        //memcpy(frames[currentFrame].uboScreen.mapped, &ubo, size);

        //模型的位置
        scene.update(time);
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
