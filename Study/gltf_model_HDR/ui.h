#pragma once
#include "imgui.h"
#include "vulkan/vulkan.h"
#include  "VulkanDevice.h"
#include "VulkanTools.h"
#include "Buffer.h"
#include <glm.hpp>

struct UIData
{
	std::string models[3] = { "Helmet","Ship","Dino" };
	std::string brdf[6] = { "NONE","Diff( L,N )","F ( L,H )","G ( L,V,H )","D ( H )","Specular" };
	std::string inputs[7] = { "NONE","Color","Normal","Occlusion","Emission","Metallic","Roughness" };
	bool operate[3] = { true,false,false };
	float color[4] = { 1,1,1,1 };
	int32_t equationCounter = 0;
	int32_t inputsCounter = 0;

	struct PushConstBlock {
		glm::vec2 scale;
		glm::vec2 translate;
	} pushConstBlock;
};
class ImGUI {
private:
	VkSampler sampler;
	mg::Buffer vertexBuffer;
	mg::Buffer indexBuffer;
	int32_t vertexCount = 0;
	int32_t indexCount = 0;
	VkDeviceMemory fontMemory = VK_NULL_HANDLE;
	VkImage fontImage = VK_NULL_HANDLE;
	VkImageView fontView = VK_NULL_HANDLE;
	VkPipelineCache pipelineCache;
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;
	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSet;
	mg::VulkanDevice* device;
	VkPhysicalDeviceDriverProperties driverProperties = {};

public:
	UIData data;

	ImGUI(mg::VulkanDevice* vulkanDevice);
	~ImGUI();

	void init(float width, float height);
	void initResources(VkRenderPass renderPass, VkQueue copyQueue, bool isMSAA);
	VkPipelineShaderStageCreateInfo loadShader(std::string fileName, VkShaderStageFlagBits stage, std::vector<VkShaderModule>* modules);
	void newFrame( );

	//更新 生成的 顶点数据
	void updateBuffers();
	void updateUI(float frameTimer, glm::vec2 mousePos, bool left, bool right, bool middle);
	void drawFrame(VkCommandBuffer commandBuffer);

};
