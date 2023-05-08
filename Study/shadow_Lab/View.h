#pragma once
#include <vector>
#include "glm.hpp"
#include "Geo.h"

using namespace mg;

/* 
	相机-灯光,	输入-控制
*/

struct  View
{
	struct UniformBufferObject
	{
		glm::mat4 view;
		glm::mat4 proj;
		glm::mat4 lightSpace;// 将 world-Space 坐标转换到 light-Space
		glm::vec4 light;
		glm::vec3 camera;
	}data;
	/* 灯光空间 */
	struct ShadowObject
	{
		glm::mat4 lightSpace;
	}shadowData;

	/* 焦点 */
	vec3 focus;
	/* 移动范围 */
	float baseRadius;
	float radius;
	vec2 range;
	/* 水平面角 和 垂直角 <欧拉角> */
	float hAngle;
	float vAngle;
	/* 加速 */
	float acc = 1.0f;
	int frameCount = 0;
	int lastFrame = -1;

	View(vec3 focus, float baseRadius, vec2 range, VkExtent2D extent)
	{
		this->focus = focus;
		this->baseRadius = baseRadius;
		this->range = range;
		/* 投影矩阵 */
		data.proj = glm::perspective(glm::radians(45.0f), extent.width / (float)extent.height, 0.1f, 100.0f);
		data.proj[1][1] *= -1;
		/* 初始值 */
		vAngle = 45;
		hAngle = 200;
		radius = baseRadius+(range.x+range.y)/2;
		setLight();
	}
	void setPose(float radius, float hAngle, float vAngle)
	{
		/* 钳制一些数据 */
		if (hAngle > 360)hAngle -= 360;
		if (hAngle < -360)hAngle += 360;
		this->hAngle = hAngle;
		this->vAngle = glm::clamp(vAngle, 15.0f, 75.0f);
		this->radius = glm::clamp(radius, baseRadius + range.x, baseRadius + range.y);
		float arcH = glm::radians(hAngle);
		float arcV = glm::radians(vAngle);
		/* 水平面上 投影半径 */
		float projRadius = glm::cos(arcV) * radius;
		float z = glm::sin(arcV) * radius;
		float x = glm::sin(arcH) * projRadius;
		float y = glm::cos(arcH) * projRadius;
		/* 相机参数 */
		data.camera = { x,y,z };
		data.view = glm::lookAt(data.camera, focus, { 0,0,1.0f });

	}
	/* 灯光*/
	void setLight() 
	{
		vec3 lightPos = { 6, -4, 8 };
		vec3 lightDir = -glm::normalize(glm::vec3(-2,3,-9));
		vec3 upDir = { 0,0,1 };
		float lightIntensity = 0.7f;

		float lightFOV = 45;
		float lightNear = 1.0f;
		float lightFar = 100.0f;
		/* 此矩阵 决定了能 “看”到阴影的范围 */
		glm::mat4 depthProjectionMatrix = glm::perspective(glm::radians(lightFOV), 1.0f, lightNear, lightFar);
		glm::mat4 depthViewMatrix = glm::lookAt(lightPos, glm::vec3(0.0f),upDir);//
		/* 平行光 */
		glm::mat4 depthModelMatrix = glm::mat4(1.0f);
		depthProjectionMatrix[1][1] = -1;
		shadowData.lightSpace = depthProjectionMatrix * depthViewMatrix;// *depthModelMatrix;
		/* 用于测试 */
		vec3 pt = { 1,1,1 };
		vec4 px = depthProjectionMatrix * vec4(pt, 1);//matrix 显示的是 ->
		vec4 py = depthViewMatrix * vec4(pt, 1);
		vec4 pz = shadowData.lightSpace * vec4(pt, 1);

		data.light = { lightDir,lightIntensity };
		data.lightSpace = shadowData.lightSpace;
	}
	void update(float deltaTime,int op)
	{
		frameCount += 1;
		float deltaHAngle = 0;
		float deltaRadius = 0;
		float deltaVAngle = 0;
		float speedHAngle = 360;
		float speedVAngle = 180;
		float speedRadius = 50.0f;
		/* 做加速度 */
		if (op > 0)
		{
			acc = acc + deltaTime;
			lastFrame = frameCount;
		}
		else if(frameCount-lastFrame>1000)
		{
			acc = 1.0f;
		}
		acc = glm::clamp(acc, 1.0f, 2.0f)*0.1f;
		switch (op)
		{
		case 'a'://左右
			deltaHAngle = deltaTime * speedHAngle;
			break;
		case 'd':
			deltaHAngle = -deltaTime * speedHAngle;
			break;
		case 'w'://前后
			deltaRadius = -deltaTime * speedRadius;
			break;
		case 's':
			deltaRadius = deltaTime * speedRadius;
			break;
		case 'z'://上下
			deltaVAngle = deltaTime * speedVAngle;
			break;
		case 'x':
			deltaVAngle = -deltaTime * speedVAngle;
			break;
		default:
			//acc = 1.0f;
			break;
		}
		hAngle += deltaHAngle*acc;
		vAngle += deltaVAngle*acc;
		radius += deltaRadius*acc;
		setPose(radius, hAngle, vAngle);
	}
};
