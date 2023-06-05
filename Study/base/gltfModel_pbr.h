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

		VkPipelineLayout pipelineLayout;

		struct Pipelines
		{
			VkPipeline pbr;
			VkPipeline pbrDoubleSided;
			VkPipeline pbrAlphaBlend;
		} pipelines;
		VkPipeline boundPipeline = VK_NULL_HANDLE;

		struct DescriptorSetLayouts {
			VkDescriptorSetLayout scene;
			VkDescriptorSetLayout material;
			VkDescriptorSetLayout node;
		} descriptorSetLayouts;

		std::vector<VkDescriptorSet> descSet_Scene;
		std::vector<UniformBufferSet> uniformBuffers;

		enum PBRWorkflows { PBR_WORKFLOW_METALLIC_ROUGHNESS = 0, PBR_WORKFLOW_SPECULAR_GLOSINESS = 1 };

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

		gltfModel_pbr(mg::VulkanDevice* vulkanDevice,uint32_t swapchainImgCount,PbrEnv* env, ModelInfo modelInfo);
		void setup(VkDescriptorPool pool);
		void clean();
		void getSpecRender(geos::gltfPbrRender_spec* render);
		void renderNode(VkCommandBuffer cmd, vkglTF::Node* node, uint32_t cbIndex, vkglTF::Material::AlphaMode alphaMode);
		//管线提前绑定
		void renderNode_ByXPipe(VkCommandBuffer cmd,vkglTF::Node* node, VkPipelineLayout pipeLayout, VkPipelineStageFlags stageFlags,vkglTF::Material::AlphaMode alphaMode);
		void load_gltf();
		void prepareUniformBuffers();
		void updateUniformBuffers(uint32_t currentBuffer,glm::mat4 proj, glm::mat4 view, glm::vec3 camPos);
		void setupDescriptors(VkDescriptorPool descriptorPool);
		/* 到根节点的矩阵 的ubo */
		void setupNodeDescriptorSet(vkglTF::Node* node, VkDescriptorPool descriptorPool);	
		void preparePipelines(VkRenderPass renderPass);
		VkPipelineShaderStageCreateInfo loadShader(VkDevice device, std::string filename, VkShaderStageFlagBits stage);
		//傻叉头文件问题
		void createBuffer(mg::VulkanDevice* device, vks::saBuffer* buffer, VkBufferUsageFlagBits usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size);
		void destroyBuffer(vks::saBuffer* buffer);
};

}