#pragma once
#include <vector>
#include "glm.hpp"
#include "camera.hpp"
#include "Geo.h"

using namespace mg;

/*
	���-�ƹ�,	����-����
*/
#define LIGHT_COUNT 3
struct  View
{
	Camera camera;
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
		glm::ivec4 debug;
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
		camera.type = Camera::CameraType::lookat;

		camera.setPerspective(45.0f, (float)extent.width / (float)extent.height, 0.1f, 256.0f);
		camera.rotationSpeed = 0.25f;
		camera.movementSpeed = 0.1f;
		camera.setPosition({ 0.0f, 0.0f, 10.0f });
		camera.setRotation({ 0.0f, 0.0f, 0.0f });

		glm::vec3 camPos = glm::vec3(
			-camera.position.z * sin(glm::radians(camera.rotation.y)) * cos(glm::radians(camera.rotation.x)),
			-camera.position.z * sin(glm::radians(camera.rotation.x)),
			camera.position.z * cos(glm::radians(camera.rotation.y)) * cos(glm::radians(camera.rotation.x))
		);
		data.proj = camera.matrices.perspective;
		data.view = camera.matrices.view;
		data.camera = glm::vec4(camPos, 1.0f);
		setLight();
	}

	/* �ƹ�*/
	void setLight()
	{
		vec3 lightPos = { 6,-4,8 };
		vec3 targetPos = { 0,0,0 };
		vec3 upDir = { 0,0,1 };

		float lightFOV = 75;
		float lightNear = 1.0f;
		float lightFar = 100.0f;

		/* ���� �������� ����������Ӱ�ķ�Χ */
		glm::mat4 shadowProj = glm::perspective(glm::radians(lightFOV), 1.0f, lightNear, lightFar);
		glm::mat4 shadowView;
		glm::mat4 shadowModel = glm::mat4(1.0f);
		glm::vec4 color;
		Light light;
		//shadowProj[1][1] = -1;
		for (int i = 0; i < LIGHT_COUNT; i++)
		{
			switch (i)
			{
			case 0:
				lightPos = { 6,-8,-4 };
				color = { 1.0f,0.33f,0.33f,0.6f };
				break;
			case 1:
				lightPos = { 2,-8,-3 };
				color = { 0.33f,1.0f,0.33f,0.6f };
				break;
			case 2:
				lightPos = { -2,-8,-3 };
				color = { 0.33f,0.33f,1.0f,0.6f };
				break;
			}
			shadowView = glm::lookAt(lightPos, targetPos, upDir);
			light.mvp = shadowProj * shadowView * shadowModel;
			light.vec = { glm::normalize(targetPos - lightPos),0 };//ƽ�й� ����
			light.color = color;
			data.lights[i] = light;
		}
	}
	void update(float deltaTime, int op)
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
		else if (frameCount - lastFrame > 1000)
		{
			acc = 1.0f;
		}
		acc = glm::clamp(acc, 1.0f, 2.0f) * 0.1f;
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
		hAngle += deltaHAngle * acc;
		vAngle += deltaVAngle * acc;
		radius += deltaRadius * acc;
		//setPose(radius, hAngle, vAngle);
	}
};
