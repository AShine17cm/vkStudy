#include "misc.h"
#include "mg.h"

#include "RenderPassHub.h"
#include "PipelineHub.h"
#include "Frame.h"
#include "Scene.h"
#include "Input.h"
#include "ui.h"
#include "PerObjectData.h"

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
    RenderPassHub passHub;
    std::vector<Frame> frames;
    PipelineHub piHub;

    /* 渲染流程 */

    VkCommandPool commandPool;
    VkDescriptorPool descriptorPool;
    /*geos*/
    Scene scene;
    /* resource */
    Resource resource;
    Input* input;
    ImGUI* imGui;
    float deltaTime = 0.01f;
    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    VkFormat depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;

    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan PBR", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<GeneralApp*>(glfwGetWindowUserPointer(window));
    }

    void initVulkan() {
        input = new Input();
        createInstance();
        setupDebugMessenger(instance,&debugMessenger);

        MG_CHECK_RESULT(glfwCreateWindowSurface(instance, window, nullptr, &surface));          //Surface
        mg::surfaces::pickPhysicalDevice(instance, surface, deviceExtensions, &physicalDevice); //显卡 graphics-present format-mode ex
        createLogicalDevice();

        passHub.init(vulkanDevice,surface, depthFormat);

        createGraphicsPipeline();
        /* geos */

        scene.prepare(vulkanDevice,passHub.extent,input, passHub.swapchain->imageCount);

        createResources();

        prepareImGui();
        scene.prepareStep2(descriptorPool, passHub.msaaTarget.renderPass,&frames,&imGui->data);
 
        passHub.createFrameBuffers(&resource);//为了 共享一个 depth-tex
        //准备 hdr 的描述符
        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
             frames[i].add_hdr(device, &passHub);
        }

        createCommandBuffers();
        createSyncObjects();
    }
    void prepareImGui()
    {
        imGui = new ImGUI(vulkanDevice);
        imGui->init((float)WIDTH, (float)HEIGHT);
        imGui->initResources(passHub.uiPass, vulkanDevice->graphicsQueue, false);
    }
    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            input->Process(window);
            drawFrame( );
            glfwPollEvents();
        }

        vkDeviceWaitIdle(device);
    }

    void cleanup() {

        delete imGui;
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
        /* 打开 features:geometryShader */
        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.geometryShader = VK_TRUE;
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceFeatures.sampleRateShading = VK_TRUE;
        deviceFeatures.wideLines = VK_TRUE;
        vulkanDevice = new mg::VulkanDevice(physicalDevice);
        vulkanDevice->createLogicalDevice(deviceFeatures, deviceExtensions, nullptr);//queueCI, command-pool
        //
        device = vulkanDevice->logicalDevice;
        commandPool = vulkanDevice->commandPool;
        graphicsQueue = vulkanDevice->graphicsQueue;
        presentQueue = vulkanDevice->presentQueue;
    }

    void createGraphicsPipeline() {

        piHub.prepare(device,&passHub,sizeof(geos::PerObjectData),sizeof(geos::HdrPushData));
    }

    void createResources() 
    {
        /* 没统计 */
        uint32_t countUBO = MAX_FRAMES_IN_FLIGHT * (256);
        uint32_t countSampler = MAX_FRAMES_IN_FLIGHT * (256);
        /* Pool */
        std::vector<VkDescriptorPoolSize> poolSizes =
        {
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,countUBO},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,countSampler}
        };
        uint32_t maxSets = MAX_FRAMES_IN_FLIGHT * (32);
        mg::descriptors::createDescriptorPool(poolSizes.data(), poolSizes.size(), device, &descriptorPool, nullptr, maxSets);
        /* Tex 和 depth-Tex */
        resource.prepare(vulkanDevice,passHub.extent);

        VkDeviceSize bufferSizes[4] = {
            sizeof(View::UniformBufferObject)
            };
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
    void updateScene(uint32_t currentFrame,uint32_t imageIndex)
    {
        Frame* pFrame = &frames[currentFrame];
        pFrame->update(device, &resource);
        static auto startTime = std::chrono::high_resolution_clock::now();
        static float lastTime = 0;
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        /* 计算帧之间的时间差 */
        deltaTime = time - lastTime;
        lastTime = time;
        /* 更新 模型和Camera */
        scene.update(pFrame,imageIndex, time, deltaTime);
    }
    /* 帧级别的渲染 */
    void recordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex) 
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        MG_CHECK_RESULT(vkBeginCommandBuffer(cmd, &beginInfo));

        std::array<VkClearValue, 3> clears{};

        Frame* frame = &frames[currentFrame];
        uint32_t dstSet = 0;    //场景矩阵 信息
        if (true) 
        {
            clears[0].depthStencil = { 1.0f,0 };
            /* 阴影渲染 */
            mg::batches::BeginRenderpass(cmd, passHub.shadowPass, passHub.shadow_FB, clears.data(), 1, passHub.shadowExtent);
            mg::batches::SetViewport(cmd, passHub.shadowExtent);

            /* 在raster中开启 depthBiasEnable, 并包含于dynamicStates */
            float depthBiasConstant = 1.25f;    //Constant depth bias factor (always applied)
            float depthBiasSlope = 1.75f;       //和 三角面在视口中的斜率相关
            vkCmdSetDepthBias(cmd,depthBiasConstant,0.0f,depthBiasSlope);// Set depth bias (aka "Polygon offset")
            genShadowMap(cmd, frame);
            vkCmdEndRenderPass(cmd);
        }
        if (true) 
        {
            clears[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
            clears[1].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
            clears[2].depthStencil = { 1.0f,0 };
            /* 正常渲染 */
            mg::batches::BeginRenderpass(cmd,passHub.msaaTarget.renderPass,passHub.msaaTarget.frameBuffer,clears.data(), 3, passHub.extent);
            mg::batches::SetViewport(cmd, passHub.extent);
            //scene.draw(cmd,nullptr, -1);//调试点
            /* pbr 渲染 */
            /* 场景信息+ShadowMap */
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.piLayout_shadow_h, dstSet, 1, &frame->scene_shadow_h, 0, nullptr);
            dstSet = 1;
            //自定义 基础 Pbr 渲染
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.pi_pbr_basic);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.piLayout_pbrBasic, dstSet, 1, &frame->pbrBasic_bg, 0, nullptr);
            scene.draw_gltf_ByXPipe(cmd, piHub.piLayout_pbrBasic, 3,-1);    //海岛
            //vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.piLayout_pbrBasic, dstSet, 1, &frame->pbrBasic, 0, nullptr);
            //scene.draw_gltf_ByXPipe(cmd, piHub.piLayout_pbrBasic, 2);     //飞艇
            //scene.draw_gltf_ByXPipe(cmd, piHub.piLayout_pbrBasic, 1);     //恐龙
            // 
            //自定义的 IBL
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.pi_pbr_IBL);
            dstSet = 1;//pbr 环境
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.piLayout_pbrEnv, dstSet, 1, &frame->pbr_Env, 0, nullptr);
            dstSet = 2;//pbr 纹理
            //头盔  金属流
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.piLayout_pbrIBL, dstSet, 1, &frame->pbr_IBL_helmet, 0, nullptr);
            scene.draw_gltf_ByXPipe(cmd, piHub.piLayout_pbrIBL, 0, -1);
            //恐龙  高光流
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.piLayout_pbrIBL, dstSet, 1, &frame->pbr_IBL_dino, 0, nullptr);
            scene.draw_gltf_ByXPipe(cmd, piHub.piLayout_pbrIBL, 2,-1);
            //飞艇 2部分
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.piLayout_pbrIBL, dstSet, 1, &frame->pbr_IBL_ship1, 0, nullptr);
            scene.draw_gltf_ByXPipe(cmd, piHub.piLayout_pbrIBL, 1, 1);//第一个节点
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.piLayout_pbrIBL, dstSet, 1, &frame->pbr_IBL_ship2, 0, nullptr);
            scene.draw_gltf_ByXPipe(cmd, piHub.piLayout_pbrIBL, 1, 2);

            //scene.draw_gltf(cmd,imageIndex,0);//头盔 金属流
            //scene.draw_gltf(cmd, imageIndex, 1);//飞艇
            //scene.draw_gltf(cmd, imageIndex,2);//恐龙
            vkCmdEndRenderPass(cmd);

            
        }
        /* 曝光 将高动态 映射到 低动态的帧，分离高亮部分 */
        if(true)
        {
            clears[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
            clears[1].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
            mg::batches::BeginRenderpass(cmd, passHub.offscreen.renderPass, passHub.offscreen.frameBuffer, clears.data(), 2, passHub.extent);
            mg::batches::SetViewport(cmd, passHub.extent);
            dstSet = 0; //正常的选结果 作为输入
            geos::HdrPushData hdrData = { {imGui->data.exposure,imGui->data.brightThreshold,0,0},{0,0,0,0} };
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.piLayout_hdr_offscreen, dstSet, 1, &frame->hdr_offscreen, 0, nullptr);
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.pi_hdr_offscreen);
            vkCmdPushConstants(cmd, piHub.piLayout_hdr_offscreen, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(geos::HdrPushData), &hdrData);//曝光度
            vkCmdDraw(cmd, 6, 1, 0, 0);
            vkCmdEndRenderPass(cmd);
        }
        //做Bloom, Blend 模式
        if (true)
        {
            clears[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
            mg::batches::BeginRenderpass(cmd, passHub.bloomPass.renderPass, passHub.bloomPass.frameBuffer, clears.data(), 1, passHub.extent);
            mg::batches::SetViewport(cmd, passHub.extent);
            dstSet = 0; //正常的渲染结果 作为输入
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.piLayout_hdr_bloom, dstSet, 1, &frame->hdr_bloom, 0, nullptr);
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.pi_hdr_bloom);

            geos::HdrPushData first = { {imGui->data.blurScale,imGui->data.blurStength,0,0},{0,0,0,0} };// 第一次
            vkCmdPushConstants(cmd, piHub.piLayout_hdr_bloom, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(geos::HdrPushData), &first);
            vkCmdDraw(cmd, 6, 1, 0, 0);

            //geos::HdrPushData second = { {imGui->data.blurScale,imGui->data.blurStength,0,0},{1,0,0,0} };// 第二次
            //vkCmdPushConstants(cmd, piHub.piLayout_hdr_bloom, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(geos::HdrPushData), &second);
            //vkCmdDraw(cmd, 6, 1, 0, 0);

            vkCmdEndRenderPass(cmd);
        }
        //输出 低动态帧
        if (true)
        {
            clears[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
            mg::batches::BeginRenderpass(cmd, passHub.renderPass, passHub.swapChainFramebuffers[imageIndex], clears.data(), 1, passHub.extent);
            mg::batches::SetViewport(cmd, passHub.extent);
            dstSet = 0;
            geos::HdrPushData hdrData = { {1.0f,0,0,0},{0,0,0,0} };
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.piLayout_ldr, dstSet, 1, &frame->ldr, 0, nullptr);
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.pi_ldr);
            vkCmdPushConstants(cmd, piHub.piLayout_ldr, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(geos::HdrPushData), &hdrData);
            vkCmdDraw(cmd, 6, 1, 0, 0);

            if (imGui->data.bloom)
            {
                dstSet = 0;//输入一样 image
                vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.piLayout_blend, dstSet, 1, &frame->blend, 0, nullptr);
                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.pi_blend);
                vkCmdPushConstants(cmd, piHub.piLayout_ldr, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(geos::HdrPushData), &hdrData);
                vkCmdDraw(cmd, 6, 1, 0, 0);
            }

            imGui->drawFrame(cmd);

            vkCmdEndRenderPass(cmd);
        }

        MG_CHECK_RESULT(vkEndCommandBuffer(cmd));
    }
    void genShadowMap(VkCommandBuffer cmd, Frame* frame)
    {
        //gltf 有自己的 vertexInputAttribute,需要自己的管线
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.pi_shadow_gltf);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, piHub.piLayout_shadow, 0, 1, &frame->shadow_ubo, 0, nullptr);
        scene.draw_gltf_ByXPipe(cmd, piHub.piLayout_shadow, 0,-1);
        scene.draw_gltf_ByXPipe(cmd, piHub.piLayout_shadow, 1,-1);
        scene.draw_gltf_ByXPipe(cmd, piHub.piLayout_shadow, 2,-1);
        scene.draw_gltf_ByXPipe(cmd, piHub.piLayout_shadow, 3,-1);
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

        imGui->updateUI(deltaTime, input->mousePre, input->mb_key==0, input->mb_key==1, input->mb_key==2);
        ImGuiIO& io= ImGui::GetIO();
        input->uiMask = io.WantCaptureMouse;//鼠标在 UI 上
        imGui->newFrame();
        imGui->updateBuffers();

        updateScene(currentFrame, imageIndex);

        vkResetFences(device, 1, &inFlightFences[currentFrame]);
        //vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);

        recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores =  waitSemaphores;
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

        vkQueueWaitIdle(vulkanDevice->graphicsQueue);//需要解决 ImGUI 更新顶点的问题
    }


};

int main() {

    GeneralApp app;
    try 
    {
        /* 主逻辑 */
        app.run();
    }
    catch (const std::exception& e) 
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
