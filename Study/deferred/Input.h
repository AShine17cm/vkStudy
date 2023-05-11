#pragma once
#include "GLFW/glfw3.h"
/* 处理 键盘输入 */
struct Input
{
    bool flipShadows = false;
    bool flip_Deferred = false;
    bool displayShadowmap = true;
    int frame_op = -1;

    int opKey;
    int funcKey_;
    float lastTime;
	void Process(GLFWwindow* window) 
    {
        static auto startTime = std::chrono::high_resolution_clock::now();
        static float lastTime = 0;
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime- startTime).count();
        float deltaTime = time - lastTime;
        /* 检查按键 */
        opKey = -1;
        frame_op = -1;
        for (int i = 0; i < 26; i++) {
            if (glfwGetKey(window, GLFW_KEY_A + i) == GLFW_PRESS)
            {
                opKey = 'a' + i;
                break;
            }
        }
        if (opKey > 0)return;
        int funcKey = -1;
        /* 功能键 按下? */
        for (int i = 0; i < 10; i++) {
            if (glfwGetKey(window, GLFW_KEY_0 + i) == GLFW_PRESS)
            {
                this->lastTime = time;
                funcKey_ = GLFW_KEY_0+ i;
                break;
            }
        }
        if (funcKey_ < 0)return;
        /* 功能键 释放 */
        for (int i = 0; i < 10; i++) {
            if (glfwGetKey(window, funcKey_) == GLFW_RELEASE)
            {
                funcKey = funcKey_;
                funcKey_ = -1;
                break;
            }
        }

        switch (funcKey)
        {
        case GLFW_KEY_1:
            flip_Deferred = true;
            break;
        case GLFW_KEY_2:
            flipShadows = true;
            break;
        case GLFW_KEY_3:
            displayShadowmap = !displayShadowmap;
            break;
        case GLFW_KEY_4:
            frame_op = funcKey - GLFW_KEY_0;
            break;
        }
	}
};