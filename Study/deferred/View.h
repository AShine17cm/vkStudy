#pragma once
#include <vector>
#include "glm.hpp"
#include "Geo.h"

using namespace mg;

/* 
	���-�ƹ�,	����-����
*/
#define LIGHT_COUNT 3
struct  View
{
	struct Light
	{
		glm::mat4 mvp;
		glm::vec4 vec;		//w ƽ�й� ?
		glm::vec4 color;	//a ǿ��	
	};
	struct UniformBufferObject
	{
		glm::mat4 proj;
		glm::mat4 view;
		glm::vec4 camera;
		glm::ivec4 debugShadow = { 1,1,1,0 };//ȫ��3����Ӱ
		glm::ivec4 debugDeferred = { 0,0,0,0 };
		Light lights[LIGHT_COUNT];
	}data;

	/* ���� */
	vec3 focus;
	/* �ƶ���Χ */
	float baseRadius;
	float radius;
	vec2 range;
	/* ˮƽ��� �� ��ֱ�� <ŷ����> */
	float hAngle;
	float vAngle;
	/* ���� */
	float acc = 1.0f;
	int frameCount = 0;
	int lastFrame = -1;

	View(vec3 focus, float baseRadius, vec2 range, VkExtent2D extent)
	{
		this->focus = focus;
		this->baseRadius = baseRadius;
		this->range = range;
		/* ͶӰ���� */
		data.proj = glm::perspective(glm::radians(45.0f), extent.width / (float)extent.height, 0.1f, 100.0f);
		data.proj[1][1] *= -1;
		data.debugShadow = { 1,1,1,1 };
		/* ��ʼֵ */
		vAngle = 45;
		hAngle = 200;
		radius = baseRadius+(range.x+range.y)/2;
		setLight();
	}
	void setPose(float radius, float hAngle, float vAngle)
	{
		/* ǯ��һЩ���� */
		if (hAngle > 360)hAngle -= 360;
		if (hAngle < -360)hAngle += 360;
		this->hAngle = hAngle;
		this->vAngle = glm::clamp(vAngle, 15.0f, 75.0f);
		this->radius = glm::clamp(radius, baseRadius + range.x, baseRadius + range.y);
		float arcH = glm::radians(hAngle);
		float arcV = glm::radians(vAngle);
		/* ˮƽ���� ͶӰ�뾶 */
		float projRadius = glm::cos(arcV) * radius;
		float z = glm::sin(arcV) * radius;
		float x = glm::sin(arcH) * projRadius;
		float y = glm::cos(arcH) * projRadius;
		/* ������� */
		data.camera = { x,y,z,1.0f };
		data.view = glm::lookAt({x,y,z}, focus, { 0,0,1.0f });

	}
	/* �ƹ�*/
	void setLight() 
	{
		vec3 lightPos = { 6,-4,8 };
		vec3 targetPos = { 0,0,0 };
		vec3 upDir = { 0,0,1 };

		float lightFOV = 60;
		float lightNear = 1.0f;
		float lightFar = 100.0f;

		/* ���� �������� ����������Ӱ�ķ�Χ */
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
			light.vec = { glm::normalize(targetPos- lightPos),0 };//ƽ�й� ����
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
		/* �����ٶ� */
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
		case 'a'://����
			deltaHAngle = deltaTime * speedHAngle;
			break;
		case 'd':
			deltaHAngle = -deltaTime * speedHAngle;
			break;
		case 'w'://ǰ��
			deltaRadius = -deltaTime * speedRadius;
			break;
		case 's':
			deltaRadius = deltaTime * speedRadius;
			break;
		case 'z'://����
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
