#pragma once
#include <vector>
#include "glm.hpp"
#include "Geo.h"

using namespace mg;

/* 
	相机-灯光,	输入-控制
*/
#define LIGHT_COUNT 3
struct  View
{
	struct Light
	{
		glm::mat4 mvp;
		glm::vec4 vec;		//w 平行光 ?
		glm::vec4 color;	//a 强度	
	};
	struct UniformBufferObject
	{
		glm::mat4 proj;
		glm::mat4 view;
		glm::vec4 camera;
		glm::ivec4 debugShadow = { 1,1,1,0 };//全部3个阴影
		glm::ivec4 debugDeferred = { 0,0,0,0 };
		Light lights[LIGHT_COUNT];
	}data;

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
		data.debugShadow = { 1,1,1,1 };
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
		data.camera = { x,y,z,1.0f };
		data.view = glm::lookAt({x,y,z}, focus, { 0,0,1.0f });

	}
	/* 灯光*/
	void setLight() 
	{
		vec3 lightPos = { 6,-4,8 };
		vec3 targetPos = { 0,0,0 };
		vec3 upDir = { 0,0,1 };

		float lightFOV = 60;
		float lightNear = 1.0f;
		float lightFar = 100.0f;

		/* 矩阵 决定了能 “看”到阴影的范围 */
		glm::mat4 shadowProj = glm::perspective(glm::radians(lightFOV), 1.0f, lightNear, lightFar);
		glm::mat4 shadowView;
		glm::mat4 shadowModel = glm::mat4(1.0f);
		glm::vec4 color;
		Light light;
		shadowProj[1][1] = -1;
		for (int i = 0; i < LIGHT_COUNT; i++)
		{
			switch (i)
			{
			case 0:
				lightPos = { 6,-4,8 };
				color = { 1.0f,0.33f,0.33f,0.6f };
				break;
			case 1:
				lightPos = { 8,2,12 };
				color = { 0.33f,1.0f,0.33f,0.6f };
				break;
			case 2:
				lightPos = { 2,5,8 };
				color = { 0.33f,0.33f,1.0f,0.6f };
				break;
			}
			shadowView = glm::lookAt(lightPos, targetPos, upDir);
			light.mvp = shadowProj * shadowView * shadowModel;
			light.vec = { glm::normalize(targetPos- lightPos),0 };//平行光 方向
			light.color = color;
			data.lights[i] = light;
		}
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
