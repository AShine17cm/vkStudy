#pragma once
#include "GLFW/glfw3.h"
/* ���� �������� */
struct Input
{
    int opKey;
    int funcKey,funcKey_; //funcKey_ �Ǵ�����Ĺ��ܼ�, ��Ƶ������
    float lastTime;
    float clearTimer;
    int GetKey() {
        if (opKey > 0)return opKey;
        if (funcKey > 0)return funcKey- GLFW_KEY_0;
        return -1;
    }
	void Process(GLFWwindow* window) 
    {
        static auto startTime = std::chrono::high_resolution_clock::now();
        static float lastTime = 0;
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime- startTime).count();
        float deltaTime = time - lastTime;
        /* ��鰴�� */
        opKey = -1;
        for (int i = 0; i < 26; i++) {
            if (glfwGetKey(window, GLFW_KEY_A + i) == GLFW_PRESS)
            {
                opKey = 'a' + i;
                break;
            }
        }
        clearTimer += deltaTime;
        if (opKey > 0)return;
        
        if (clearTimer > 0.3f) 
        {
            funcKey = -1;
            clearTimer = 0;
        }
        /* ���ܼ� ����? */
        for (int i = 0; i < 10; i++) {
            if (glfwGetKey(window, GLFW_KEY_0 + i) == GLFW_PRESS)
            {
                this->lastTime = time;
                funcKey_ = GLFW_KEY_0+ i;
                clearTimer = 0;
                break;
            }
        }
        if (funcKey_ < 0)return;
        /* ���ܼ� �ͷ� */
        for (int i = 0; i < 10; i++) {
            if (glfwGetKey(window, funcKey_) == GLFW_RELEASE)
            {
                funcKey = funcKey_;
                funcKey_ = -1;
                break;
            }
        }
	}
};