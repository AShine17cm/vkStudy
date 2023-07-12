#pragma once
#include "GLFW/glfw3.h"
/* 处理 鼠标 键盘输入 */
struct Input
{
    bool uiMask = false;
    bool flipShadows = false;
    /* 鼠标数据 */
    int mb_key = -1;
    glm::vec2 mouseStart;
    glm::vec2 mousePre;
    int32_t dx=0, dy=0;

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
        //鼠标 0-左 1-右 2-中键
        if (mb_key==-1)
        {
            dx = dy = 0;
            int mb_0 = glfwGetMouseButton(window, 0);
            int mb_1 = glfwGetMouseButton(window, 1);
            int mb_2 = glfwGetMouseButton(window, 2);
            if (mb_0 == 1)
            {
                mb_key = 0;
            }
            if (mb_1 == 1)
            {
                mb_key = 1;
            }
            if (mb_2 == 1)
            {
                mb_key = 2;
            }
            if (mb_key > -1)
            {
                double xPos, yPos;
                glfwGetCursorPos(window, &xPos, &yPos);
                mouseStart = glm::vec2(xPos, yPos);
                mousePre = mouseStart;
                //glfwSetCursorPosCallback(window, mouse_callback);
            }
        }
        else
        {
           int mb_state=  glfwGetMouseButton(window, mb_key);
           if (mb_state == 0)
           {
               mb_key = -1;
           }
           else//持续按下状态
           {
               double xPos, yPos;
               glfwGetCursorPos(window, &xPos, &yPos);
               dx = (int32_t)mousePre.x - (int32_t)xPos;
               dy = (int32_t)mousePre.y - (int32_t)yPos;
               mousePre = glm::vec2(xPos, yPos);
           }
        }


        /* 检查按键 */
        opKey = -1;
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
            flipShadows = true;
            //flipEquation = true;
            funcKey = -1;
            break;
        case GLFW_KEY_2:
            //flipViewInputs = true;
            //funcKey = -1;
            break;
        case GLFW_KEY_3:
            //funcKey = -1;

            break;
        case GLFW_KEY_4:
            break;
        }
	}
    void mouse_callback(GLFWwindow* window, double xPos, double yPos)
    {

    }
};