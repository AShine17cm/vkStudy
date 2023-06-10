#include <chrono>
#include "gltfModel_pbr.h"
#include "macros.h"
#include "PerObjectData.h"

namespace vks 
{
	/*	imageCount 做初始化
		后续的 buffer索引需要走 imageIndex
	*/
	gltfModel_pbr::gltfModel_pbr(mg::VulkanDevice* vulkanDevice,uint32_t swapchainImgCount,PbrEnv* env, gltfModel_pbr::ModelInfo modelInfo,bool msaa)
	{
		this->vulkanDevice = vulkanDevice;
		this->device = vulkanDevice->logicalDevice;
		this->modelInfo = modelInfo;
		this->env = env;
		this->shaderValuesParams.prefilteredCubeMipLevels = env->prefilteredCubeMipLevels;
		this->msaa = msaa;

		uniformBuffers.resize(swapchainImgCount);
		descSet_Scene.resize(swapchainImgCount);

		load_gltf();
		prepareUniformBuffers();
	}
	void gltfModel_pbr::clean()
	{
		scene.destroy(device);

		for (UniformBufferSet& buffer : uniformBuffers) 
		{
			destroyBuffer(&buffer.params);
			destroyBuffer(&buffer.scene);
			destroyBuffer(&buffer.skybox);
		}
	}

	void gltfModel_pbr::getPrimitives(std::vector<vkglTF::Node*>* nodes, std::vector<vkglTF::Primitive*>* prims, std::vector<vkglTF::Node*>* nodeX)
	{
		for (int i = 0; i < (*nodes).size(); i++)
		{
			vkglTF::Node* node = (*nodes)[i];

			if (node->mesh != nullptr)
			{
				for (auto pri : node->mesh->primitives)
				{
					(*prims).push_back(pri);
					(*nodeX).push_back(node);
				}
			}
		}
		for (int i = 0; i < (*nodes).size(); i++)
		{
			vkglTF::Node* node = (*nodes)[i];

			if (node->children.size()>0)
			{
				getPrimitives(&(node->children), prims,nodeX);
			}
		}
	}
	//获取加载到的贴图的信息 第一个有料的节点
	void gltfModel_pbr::getSpecRender(geos::gltfPbrRender_spec* render,int idxNode)
	{
		std::vector<vkglTF::Primitive*> primitives;
		std::vector<vkglTF::Node*> nodes;
		getPrimitives(&scene.nodes, &primitives,&nodes);

		if (primitives.size()<idxNode) 
		{
			std::cout << "gltfModel_pbr 未找到 mesh" << std::endl;
			return;
		}
		vkglTF::Primitive* primitive = primitives[idxNode];
		render->normalImg = &primitive->material.normalTexture->descriptor;
		render->ocImg = &primitive->material.occlusionTexture->descriptor;

		//高光贴图
		if (primitive->material.emissiveTexture == nullptr)
		{
			render->mat.emissiveTextureSet = -1;
		}
		else
		{
			render->mat.emissiveTextureSet = 0;
			render->emissiveImg = &primitive->material.emissiveTexture->descriptor;
		}
		//到根节点的矩阵
		//render->toRoot = node->mesh->uniformBuffer.descriptorSet;
		render->model = shaderValuesScene.model * nodes[idxNode]->getMatrix();
		geos::PbrMaterial* mat = &render->mat;

		mat->emissiveFactor = primitive->material.emissiveFactor;
		mat->alphaMask = static_cast<float>(primitive->material.alphaMode == vkglTF::Material::ALPHAMODE_MASK);
		mat->alphaMaskCutoff = primitive->material.alphaCutoff;

		//金属流
		if (render->isMetallic)//metallic
		{
			render->colorImg = &primitive->material.baseColorTexture->descriptor;
			render->metalRough = &primitive->material.metallicRoughnessTexture->descriptor;

			mat->baseColorFactor = primitive->material.baseColorFactor;
			mat->metallicFactor = primitive->material.metallicFactor;
			mat->roughnessFactor = primitive->material.roughnessFactor;
		}
		//高光流
		if (!render->isMetallic)//specular
		{
			render->colorImg = &primitive->material.extension.diffuseTexture->descriptor;
			render->specImg = &primitive->material.extension.specularGlossinessTexture->descriptor;

			mat->diffuseFactor = primitive->material.extension.diffuseFactor;
			mat->specularFactor = glm::vec4(primitive->material.extension.specularFactor, 1.0f);
		}
	}
	//用提前绑定的管线 渲染(阴影) (要保证vertex的InputAttribute和管线一致属性)
	void gltfModel_pbr::renderNode_ByXPipe(VkCommandBuffer cmd, vkglTF::Node* node,VkPipelineLayout pipeLayout,VkPipelineStageFlags stageFlags, vkglTF::Material::AlphaMode alphaMode,int idxNode)
	{
		if (node->mesh) {
			for (vkglTF::Primitive* primitive : node->mesh->primitives)
			{
				if (primitive->material.alphaMode == alphaMode)
				{
					counter += 1;
					if (idxNode < 0 || counter == idxNode)
					{
						//节点矩阵
						geos::PerObjectData pod = { shaderValuesScene.model * node->getMatrix(), {0,0,0,0} };
						vkCmdPushConstants(cmd, pipeLayout, stageFlags, 0, sizeof(geos::PerObjectData), &pod);
						if (primitive->hasIndices) 
						{
							vkCmdDrawIndexed(cmd, primitive->indexCount, 1, primitive->firstIndex, 0, 0);
						}
						else 
						{
							vkCmdDraw(cmd, primitive->vertexCount, 1, 0, 0);
						}
					}
				}
			}
		};
		//递归 节点
		for (auto child : node->children)
		{
			renderNode_ByXPipe(cmd, child, pipeLayout, stageFlags, alphaMode,idxNode);
		}
	}
	void gltfModel_pbr::load_gltf( )
	{
		std::cout << "Scene::加载场景: " <<modelInfo.sceneFile.c_str() << std::endl;
		auto tStart = std::chrono::high_resolution_clock::now();
		scene.loadFromFile(modelInfo.sceneFile.c_str(), vulkanDevice, vulkanDevice->graphicsQueue);
		auto tFileLoad = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - tStart).count();
		std::cout << "Scene::加载到: " << tFileLoad << " ms" << std::endl;
	}

	void gltfModel_pbr::prepareUniformBuffers()
	{
		for (UniformBufferSet& uniformBuffer : uniformBuffers)
		{
			createBuffer(vulkanDevice, &uniformBuffer.scene, 
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(shaderValuesScene));
			createBuffer(vulkanDevice, &uniformBuffer.skybox,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(shaderValuesSkybox));
			createBuffer(vulkanDevice, &uniformBuffer.params,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(shaderValuesParams));
		}
	}
	void gltfModel_pbr::updateUniformBuffers(uint32_t currentBuffer,glm::mat4 proj,glm::mat4 view, glm::vec3 camPos)
	{
		// Scene
		shaderValuesScene.projection = proj;
		shaderValuesScene.view =view;
		float modelScale = modelInfo.modelScale;
		glm::vec3 modelTranslate = modelInfo.modelTranslate;
		float rotate = modelInfo.roate;
		glm::vec3 rotateAxis = modelInfo.rotateAxis;
		// Center and scale model
		//float scale = (1.0f / std::max(models.scene.aabb[0][0], std::max(models.scene.aabb[1][1], models.scene.aabb[2][2]))) * 0.5f;
		//glm::vec3 translate = -glm::vec3(models.scene.aabb[3][0], models.scene.aabb[3][1], models.scene.aabb[3][2]);
		//translate += -0.5f * glm::vec3(models.scene.aabb[0][0], models.scene.aabb[1][1], models.scene.aabb[2][2]);

		//场景根节点 的缩放，平移
		shaderValuesScene.model = glm::mat4(1.0f);
		shaderValuesScene.model[0][0] = modelScale;
		shaderValuesScene.model[1][1] = modelScale;
		shaderValuesScene.model[2][2] = modelScale;
		shaderValuesScene.model = glm::translate(shaderValuesScene.model, modelTranslate);
		shaderValuesScene.model = glm::rotate(shaderValuesScene.model, glm::radians(rotate), rotateAxis);
		//
		shaderValuesScene.camPos = camPos;
		// Skybox
		shaderValuesSkybox.projection = proj;
		shaderValuesSkybox.view = view;
		shaderValuesSkybox.model = glm::mat4(glm::mat3(view));

		//拷贝数据
		UniformBufferSet currentUB = uniformBuffers[currentBuffer];
		memcpy(currentUB.scene.mapped, &shaderValuesScene, sizeof(shaderValuesScene));
		memcpy(currentUB.params.mapped, &shaderValuesParams, sizeof(shaderValuesParams));
		memcpy(currentUB.skybox.mapped, &shaderValuesSkybox, sizeof(shaderValuesSkybox));
	}

	void gltfModel_pbr::createBuffer(mg::VulkanDevice* device, vks::saBuffer* buffer,VkBufferUsageFlagBits usageFlags,VkMemoryPropertyFlags memoryPropertyFlags,VkDeviceSize size)
	{
		buffer->device = device->logicalDevice;
		device->createBuffer(usageFlags, memoryPropertyFlags, size, &buffer->buffer, &buffer->memory,nullptr);
		buffer->descriptor = { buffer->buffer, 0, size };
		//if (map)
		{
			VK_CHECK_RESULT(vkMapMemory(device->logicalDevice, buffer->memory, 0, size, 0, &buffer->mapped));
		}
	}
	void gltfModel_pbr::destroyBuffer(vks::saBuffer* buffer)
	{
		if (buffer->mapped) 
		{
			vkUnmapMemory(device, buffer->memory);
			buffer->mapped = nullptr;
		}
		vkDestroyBuffer(device, buffer->buffer, nullptr);
		vkFreeMemory(device, buffer->memory, nullptr);
		buffer-> buffer = VK_NULL_HANDLE;
		buffer-> memory = VK_NULL_HANDLE;
	}
}