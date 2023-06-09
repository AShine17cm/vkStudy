#pragma once
#include "VulkanglTFModel.h"
#include "VulkanTexture.hpp"
#include "Buffer.h"
#include "saBuffer.h"
#include "PbrEnv.h"
#include "PerObjectData.h"

namespace vks 
{
	struct gltfModel_pbr
	{
		mg::VulkanDevice* vulkanDevice;
		VkDevice device;
		VkPipelineCache pipelineCache = VK_NULL_HANDLE;
		PbrEnv* env;
		bool msaa = false;
		int counter;
		struct ModelInfo
		{
			std::string sceneFile;	//模型文件
			float modelScale = 1.0f;
			glm::vec3 modelTranslate = { 0,0,0 };
			float roate = 0;
			glm::vec3 rotateAxis = { 0,1,0 };

		}modelInfo;

		vkglTF::Model scene;

		struct UniformBufferSet {
			vks::saBuffer scene;
			vks::saBuffer skybox;
			vks::saBuffer params;
		};

		struct UBOMatrices {
			glm::mat4 projection;
			glm::mat4 model;		//场景根节点 (缩放/平移)
			glm::mat4 view;
			glm::vec3 camPos;
		} shaderValuesScene, shaderValuesSkybox;

		struct shaderValuesParams {
			glm::vec4 lightDir;
			float exposure = 4.5f;
			float gamma = 2.2f;
			float prefilteredCubeMipLevels;
			float scaleIBLAmbient = 1.0f;
			float debugViewInputs = 0;
			float debugViewEquation = 0;
		} shaderValuesParams;

		std::vector<VkDescriptorSet> descSet_Scene;
		std::vector<UniformBufferSet> uniformBuffers;

		struct PushConstBlockMaterial {
			glm::vec4 baseColorFactor;
			glm::vec4 emissiveFactor;
			glm::vec4 diffuseFactor;
			glm::vec4 specularFactor;
			float workflow;
			int colorTextureSet;
			int PhysicalDescriptorTextureSet;
			int normalTextureSet;
			int occlusionTextureSet;
			int emissiveTextureSet;
			float metallicFactor;
			float roughnessFactor;
			float alphaMask;
			float alphaMaskCutoff;
		} pushConstBlockMaterial;

		gltfModel_pbr(mg::VulkanDevice* vulkanDevice,uint32_t swapchainImgCount,PbrEnv* env, ModelInfo modelInfo,bool msaa=false);

		void clean();
		void getPrimitives(std::vector<vkglTF::Node*>* nodes,std::vector<vkglTF::Primitive*>* prims, std::vector<vkglTF::Node*>* nodeX);
		void getSpecRender(geos::gltfPbrRender_spec* render,int idxNode=0);
		//管线提前绑定
		void renderNode_ByXPipe(VkCommandBuffer cmd,vkglTF::Node* node, VkPipelineLayout pipeLayout, VkPipelineStageFlags stageFlags,vkglTF::Material::AlphaMode alphaMode,int idxNode=-1);
		void load_gltf();
		void prepareUniformBuffers();
		void updateUniformBuffers(uint32_t currentBuffer,glm::mat4 proj, glm::mat4 view, glm::vec3 camPos);

		//傻叉头文件问题
		void createBuffer(mg::VulkanDevice* device, vks::saBuffer* buffer, VkBufferUsageFlagBits usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size);
		void destroyBuffer(vks::saBuffer* buffer);
};

}