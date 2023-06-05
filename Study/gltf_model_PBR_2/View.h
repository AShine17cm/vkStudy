#pragma once
#include <vector>
#include "glm.hpp"
#include "camera.hpp"
#include "Geo.h"
#include "Input.h"

using namespace mg;

/*
	相机-灯光,	输入-控制
*/
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
		glm::ivec4 debug = { 1,1,1,1 };
		Light lights[LIGHT_COUNT];
	}data;
	std::vector<glm::vec4> lightPoses;
	Camera camera;
	View(VkExtent2D extent)
	{
		camera.type = Camera::CameraType::lookat;
		camera.setPerspective(45.0f, (float)extent.width / (float)extent.height, 0.3f, 256.0f);
		camera.rotationSpeed = 0.25f;
		camera.movementSpeed = 0.1f;
		camera.setPosition({ 0.0f, 6.0f, 25.0f });
		camera.setRotation({ -8.0f, -21.0f, 0.0f });

		setup_camera();
		setLight();
	}
	void setup_camera()
	{
		glm::vec3 camPos = glm::vec3(
			-camera.position.z * sin(glm::radians(camera.rotation.y)) * cos(glm::radians(camera.rotation.x)),
			-camera.position.z * sin(glm::radians(camera.rotation.x)),
			camera.position.z * cos(glm::radians(camera.rotation.y)) * cos(glm::radians(camera.rotation.x))
		);
		data.proj = camera.matrices.perspective;
		data.view = camera.matrices.view;
		data.camera = glm::vec4(camPos, 1.0f);
	}
	/* 灯光*/
	void setLight()
	{
		vec3 lightPos = { 6,-4,8 };
		vec3 targetPos = { 0,0,0 };
		vec3 upDir = { 0,-1,0 };

		float lightFOV = 45;
		float lightNear = 1.0f;
		float lightFar = 100.0f;

		/* 矩阵 决定了能 “看”到阴影的范围 */
		glm::mat4 shadowProj = glm::perspective(glm::radians(lightFOV), 1.0f, lightNear, lightFar);
		//glm::mat4 shadowProj = glm::ortho(-10.0f,200.0f,-10.0f,200.0f, lightNear, lightFar);
		glm::mat4 shadowView;
		glm::mat4 shadowModel = glm::mat4(1.0f);
		glm::vec4 color;
		Light light;
		/*
		灯光设定 在 Vulkan 空间内, 顶点是否翻转，灯光方向是否需要翻转，需要根据 pbr.vert 函数确定
		*/
		// 灯光 在 -Y 轴上
		for (int i = 0; i < LIGHT_COUNT; i++)
		{
			switch (i)
			{
			case 0:
				lightPos = { -12,-48,24 };
				color = { 1.0f,1.0f,0.9f,0.9f };
				break;
			}
			lightPoses.push_back(glm::vec4(lightPos,1.0));
			shadowView = glm::lookAt(lightPos, targetPos, upDir);
			light.mvp = shadowProj * shadowView * shadowModel;
			light.vec = { glm::normalize(lightPos- targetPos),0 };//平行光 方向
			light.color = color;
			data.lights[i] = light;
		}
	}
	void update(Input* input, float deltaTime)
	{
		/* 鼠标转动 */
		int mb_key = input->mb_key;
		int32_t dx = input->dx;
		int32_t dy = input->dy;
		if (mb_key > -1)
		{
			//std::cout << "dx " << dx <<"dy " <<dy<< std::endl;
			switch (mb_key)
			{
			case 0:
				camera.rotate(glm::vec3(dy * camera.rotationSpeed, -dx * camera.rotationSpeed, 0.0f));
				break;
			case 1:
				camera.translate(glm::vec3(-0.0f, 0.0f, dy * .2f * camera.movementSpeed));
				break;
			case 2:
				camera.translate(glm::vec3(-dx * 0.01f, -dy * 0.01f, 0.0f));
				break;
			}
		}
		camera.update(deltaTime);
		setup_camera();
	}
};
