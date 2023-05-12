#include "misc.h"
#include "mg.h"

#include "RenderPassHub.h"
#include "PipelineHub.h"
#include "Frame.h"
#include "Scene.h"
#include "Input.h"

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
    /* ��Ⱦ���� */
    uint32_t currentFrame = 0;
    RenderPassHub passHub;
    std::vector<Frame> frames;
    PipelineHub piHub;

    /* ��Ⱦ���� */

    VkCommandPool commandPool;
    VkDescriptorPool descriptorPool;
    /*geos*/
    Scene scene;
    /* resource */
    Resource resource;
    Input* input;

    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    VkFormat depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;

    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan MG", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<GeneralApp*>(glfwGetWindowUserPointer(window));
    }

    void initVulkan() {
        createInstance();
        setupDebugMessenger(instance,&debugMessenger);

        MG_CHECK_RESULT(glfwCreateWindowSurface(instance, window, nullptr, &surface));          //Surface
        mg::surfaces::pickPhysicalDevice(instance, surface, deviceExtensions, &physicalDevice); //�Կ� graphics-present format-mode ex
        createLogicalDevice();

        passHub.init(vulkanDevice,surface,WIDTH,HEIGHT, depthFormat);

        createGraphicsPipeline();
        /* geos */
        scene.prepare(vulkanDevice,passHub.extent);

        createResources();

        passHub.createFrameBuffers(&resource);//Ϊ�� ����һ�� depth-tex

        createCommandBuffers();
        createSyncObjects();
        input = new Input();
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            input->Process(window);
            int opKey = input->GetKey();
            /* ��ֹcmd��descriptor��ռ�ã�ʹ��cache */
            for (int i = 0; i < frames.size(); i++) 
            {
                frames[i].CacheKey(opKey);
            }
            drawFrame( );
            glfwPollEvents();
        }

        vkDeviceWaitIdle(device);
    }

    void cleanup() {

        passHub.clean();
        piHub.cleanup(device);
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

    void createGraphicsPipeline() {

        piHub.prepare(device,&passHub,sizeof(Scene::PerObjectData));
    }

    void createResources() 
    {
        /* ûͳ�� */
        uint32_t countUBO = MAX_FRAMES_IN_FLIGHT * (128);
        uint32_t countSampler = MAX_FRAMES_IN_FLIGHT * (128);
        /* Pool */
        std::vector<VkDescriptorPoolSize> poolSizes =
        {
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,countUBO},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,countSampler}
        };
        uint32_t maxSets = MAX_FRAMES_IN_FLIGHT * (16);
        mg::descriptors::createDescriptorPool(poolSizes.data(), poolSizes.size(), device, &descriptorPool, nullptr, maxSets);
        /* Tex �� depth-Tex */
        resource.prepare(vulkanDevice,passHub.extent);

        VkDeviceSize bufferSizes[4] = {
            sizeof(View::UniformBufferObject),
            sizeof(View::ShadowObject),
            sizeof(Scene::InstanceData) * Scene::countInstance,
            sizeof(vec4) * 6 * Scene::countUI };
        /* Frame */
        frames.resize(MAX_FRAMES_IN_FLIGHT);
        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
        {
            frames[i].prepare(vulkanDevice, descriptorPool, &piHub,bufferSizes);
            frames[i].updateDescritors(device,&resource);
        }
    }

    void createCommandBuffers() 
    {
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        vulkanDevice->createCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY,commandBuffers.data() , MAX_FRAMES_IN_FLIGHT);
    }
    void updateScene(uint32_t currentFrame)
    {
        int opKey = input->GetKey();

        Frame* pFrame = &frames[currentFrame];
        pFrame->update(device, &resource);
        static auto startTime = std::chrono::high_resolution_clock::now();
        static float lastTime = 0;
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        /* ����֮֡���ʱ��� */
        float deltaTime = time - lastTime;
        lastTime = time;
        /* ���� ģ�ͺ�Camera */
        scene.update(pFrame, time, deltaTime, opKey);
    }
    /* ֡�������Ⱦ */
    void recordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex) 
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        MG_CHECK_RESULT(vkBeginCommandBuffer(cmd, &beginInfo));

        std::array<VkClearValue, 2> clears{};

        Frame* frame = &frames[currentFrame];
        uint32_t dstSet = 0;    //�������� ��Ϣ
        if (true) 
        {
            clears[0].depthStencil = { 1.0f,0 };
            /* ��Ӱ��Ⱦ */
            mg::batches::BeginRenderpass(cmd, passHub.shadowPass, passHub.shadow_FB, clears.data(), 1, passHub.shadowExtent);
            mg::batches::SetViewport(cmd, passHub.shadowExtent);

            /* ��raster�п��� depthBiasEnable, ��������dynamicStates */
            float depthBiasConstant = 1.25f;    //Constant depth bias factor (always applied)
            float depthBiasSlope = 1.75f;       //�� ���������ӿ��е�б�����
            vkCmdSetDepthBias(cmd,depthBiasConstant,0.0f,depthBiasSlope);// Set depth bias (aka "Polygon offset")
            genShadowMap(cmd, frame);
            vkCmdEndRenderPass(cmd);
        }
        if (true) 
        {
            clears[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
            clears[1].depthStencil = { 1.0f,0 };
            /* ������Ⱦ */
            mg::batches::BeginRenderpass(cmd,passHub.renderPass,passHub.swapChainFramebuffers[imageIndex],clears.data(), 2, passHub.extent);
            mg::batches::SetViewport(cmd, passHub.extent);
            /* ������Ϣ+ShadowMap */
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.piLayout_ubo_tex, dstSet, 1, &frame->scene_ubo_shadow, 0, nullptr);
            drawScene(cmd, frame);
            vkCmdEndRenderPass(cmd);
        }


        MG_CHECK_RESULT(vkEndCommandBuffer(cmd));
    }
    void genShadowMap(VkCommandBuffer cmd, Frame* frame)
    {
        uint32_t batchIdx =-1;
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.pi_shadow);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.piLayout_ubo, 0, 1, &frame->shadow_ubo, 0, nullptr);
        for (uint32_t i = 0; i < 4; i++) 
        {
            if (i == 2)continue;
            batchIdx = i;
            scene.draw(cmd, piHub.piLayout_ubo, batchIdx);
        }
        /* ʵ��������Ӱ*/
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.pi_shadow_instancing);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.piLayout_ubo_ubo, 1, 1, &frame->instance_ubo, 0, nullptr);
        batchIdx = 2;
        scene.draw(cmd, piHub.piLayout_ubo_ubo, batchIdx);
    }
    /* ������Ϣ+ShadowMap  ��һ�鳡�� */
    void drawScene(VkCommandBuffer cmd, Frame* frame) 
    {
        uint32_t dstSet = 0;
        uint32_t batchIdx = 0;
        /* ģ�� ���� */
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.pi_TexArray);
        dstSet = 1;             //texture
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.piLayout_ubo_tex_tex, dstSet, 1, &frame->tex_array, 0, nullptr);
        batchIdx = 0;
        scene.draw(cmd, piHub.piLayout_ubo_tex_tex, batchIdx);

        /* һ�� ������ */
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.pi_Tex3D);
        dstSet = 1;
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.piLayout_ubo_tex_tex, dstSet, 1, &frame->tex_3d, 0, nullptr);
        batchIdx = 1;
        scene.draw(cmd, piHub.piLayout_ubo_tex_tex, batchIdx);

        /* ʵ���� Cube */
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.pi_Instance);
        dstSet = 1;
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.piLayout_instance, dstSet, 1, &frame->instance_ubo_mips, 0, nullptr);
        batchIdx = 2;
        scene.draw(cmd, piHub.piLayout_instance, batchIdx);

        /* ���� */
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.pi_TexCube);
        dstSet = 1;    
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.piLayout_ubo_tex_tex, dstSet, 1, &frame->cube_map, 0, nullptr);
        batchIdx = 3;
        scene.draw(cmd, piHub.piLayout_ubo_tex_tex, batchIdx);

        /* �ذ����ͼ */
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.pi_Tex);
        dstSet = 1;    
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.piLayout_ubo_tex_tex, dstSet, 1, &frame->ground_tex, 0, nullptr);
        batchIdx = 4;
        scene.draw(cmd, piHub.piLayout_ubo_tex_tex, batchIdx);

        /* ui */
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.pi_ui);
        dstSet = 0;
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.piLayout_ubo_tex, dstSet, 1, &frame->ui_ubo_tex, 0, nullptr);
        batchIdx = 5;
        scene.draw(cmd, piHub.piLayout_ubo_tex, batchIdx);
        dstSet = 0;
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.piLayout_ubo_tex, dstSet, 1, &frame->ui_ubo_shadow, 0, nullptr);
        batchIdx = 6;
        scene.draw(cmd, piHub.piLayout_ubo_tex, batchIdx);

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



    void drawFrame( ) {
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, passHub.swapchain->swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }
        if (result == VK_ERROR_OUT_OF_DATE_KHR) 
        {
            return;
        } 

        updateScene(currentFrame);

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
        VkSwapchainKHR swapChains[] = {passHub.swapchain->swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        result = vkQueuePresentKHR(presentQueue, &presentInfo);

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }


};

int main() {

    GeneralApp app;
    try 
    {
        /* ���߼� */
        app.run();
    }
    catch (const std::exception& e) 
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}