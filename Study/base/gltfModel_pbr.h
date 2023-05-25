//#ifndef GLTF_MODEL_PBR_STUDY
//#define GLTF_MODEL_PBR_STUDY

#pragma once
#include "VulkanglTFModel.h"
#include "VulkanTexture.hpp"
#include "Buffer.h"
#include "saBuffer.h"

namespace vks 
{
	struct gltfModel_pbr
	{
		const std::string assetpath = "./../data/";
		std::string envMapFile = assetpath + "environments/papermill.ktx";
		//std::string sceneFile = "../models/Unicorn.glb";
		std::string sceneFile = "../data/models/DamagedHelmet/glTF-Embedded/DamagedHelmet.gltf";
		std::string skyFile = "../data/models/Box/glTF-Embedded/Box.gltf";
		float modelScale = 1.2f;
		glm::vec3 modelTranslate = { 0,0,0 };

		mg::VulkanDevice* vulkanDevice;
		VkDevice device;
		VkPipelineCache pipelineCache = VK_NULL_HANDLE;

		struct Textures {
			vks::TextureCubeMap environmentCube;
			vks::Texture2D empty;
			vks::Texture2D lutBrdf;
			vks::TextureCubeMap irradianceCube;
			vks::TextureCubeMap prefilteredCube;
		} textures;

		struct Models {
			vkglTF::Model scene;
			vkglTF::Model skybox;
		} models;

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
			VkPipeline skybox;
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

		struct DescriptorSets {
			VkDescriptorSet scene;
			VkDescriptorSet skybox;
		};
		std::vector<DescriptorSets> descriptorSets;

		//std::vector<VkCommandBuffer> commandBuffers;
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

		gltfModel_pbr(mg::VulkanDevice* vulkanDevice,uint32_t swapchainImgCount);
		void setup(VkDescriptorPool pool);
		void clean();
		void renderNode(VkCommandBuffer cmd, vkglTF::Node* node, uint32_t cbIndex, vkglTF::Material::AlphaMode alphaMode);
		void load_gltf();
		void loadEnvironment();
		void generateBRDFLUT();
		void generateCubemaps();
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
//#endif // !GLTF_MODEL_PBR_STUDY