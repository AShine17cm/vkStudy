#include "Geo.h"
#include "VulkanDevice.h"
#include "buffers.h"
namespace geos
{
	/*
	���㲼��  strideΪ��������Ĵ�С
	*/
	VkPipelineVertexInputStateCreateInfo Geo::getVertexInput(
		std::vector<VkVertexInputAttributeDescription>* attributes,
		std::vector<VkVertexInputBindingDescription>* bindings)
	{
		uint32_t stride = sizeof(float);
		//location	binding	fromat	offset
		/* pos normal tangent uv color */
		attributes->push_back({ 0,0,VK_FORMAT_R32G32B32_SFLOAT,0 });
		attributes->push_back({ 1,0,VK_FORMAT_R32G32B32_SFLOAT,stride * 3 });
		attributes->push_back({ 2,0,VK_FORMAT_R32G32B32_SFLOAT,stride * (3 + 3) });
		attributes->push_back({ 3,0,VK_FORMAT_R32G32_SFLOAT,stride * (3 + 3 + 3) });
		attributes->push_back({ 4,0,VK_FORMAT_R32G32B32A32_SFLOAT,stride * (3 + 3 + 3 + 2) });
		uint32_t stride_b = sizeof(Vertex);

		//binding stride rate
		bindings->push_back({ 0,stride_b,VK_VERTEX_INPUT_RATE_VERTEX });
		VkPipelineVertexInputStateCreateInfo vertexInputSCI{};
		vertexInputSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputSCI.pVertexAttributeDescriptions = attributes->data();
		vertexInputSCI.vertexAttributeDescriptionCount = attributes->size();
		vertexInputSCI.pVertexBindingDescriptions = bindings->data();
		vertexInputSCI.vertexBindingDescriptionCount = bindings->size();
		vertexInputSCI.flags = 0;

		return vertexInputSCI;
	}
	/* ��ʵ�������� ���� vertex������  */
	void Geo::getVertexInput_Instancing(
		std::vector<VkVertexInputAttributeDescription>* attributes,
		std::vector<VkVertexInputBindingDescription>* bindings,
		uint32_t instanceStride)
	{
		uint32_t stride = sizeof(float);
		//location	binding	fromat	offset
		/* pos normal tangent uv color */
		attributes->push_back({ 0,0,VK_FORMAT_R32G32B32_SFLOAT,0 });
		attributes->push_back({ 1,0,VK_FORMAT_R32G32B32_SFLOAT,stride * 3 });
		attributes->push_back({ 2,0,VK_FORMAT_R32G32B32_SFLOAT,stride * (3 + 3) });
		attributes->push_back({ 3,0,VK_FORMAT_R32G32_SFLOAT,stride * (3 + 3 + 3) });
		attributes->push_back({ 4,0,VK_FORMAT_R32G32B32A32_SFLOAT,stride * (3 + 3 + 3 + 2) });

		uint32_t stride_b = sizeof(Vertex);

		//binding stride rate
		bindings->push_back({ 0,stride_b,VK_VERTEX_INPUT_RATE_VERTEX });
		/* instancing */
		bindings->push_back({ 1,instanceStride,VK_VERTEX_INPUT_RATE_INSTANCE });
	}
	/*
	����2��buffer ����-����������
	һ����λ ��Host��Device-Local
	*/
	void Geo::prepareBuffer(mg::VulkanDevice* vulkanDevice)
	{
		uint32_t stride_v = sizeof(Vertex);
		uint32_t bufferSize = stride_v * vertices.size();
		mg::buffers::transferDataLocal(
			bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			vulkanDevice, vulkanDevice->graphicsQueue,
			&vertexBuffer,
			vertices.data());

		uint32_t stride_t = sizeof(uint32_t);
		uint32_t bufferSize_t = stride_t * triangles.size();
		mg::buffers::transferDataLocal(
			bufferSize_t, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			vulkanDevice, vulkanDevice->graphicsQueue,
			&indexBuffer,
			triangles.data());
	}
	/*
	�� ����-������ ����
	���� ������
	*/
	void Geo::drawGeo(VkCommandBuffer cmd,int instanceCount)
	{
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(cmd, 0, 1, &vertexBuffer.buffer, offsets);
		vkCmdBindIndexBuffer(cmd, indexBuffer.buffer, 0, indexType);
		vkCmdDrawIndexed(cmd, triangles.size(), instanceCount, 0, 0, 0);	//�������� �е�һ�飬�������е�һ��
		//vkCmdDraw(cmd, vertices.size(), 1, 0, 0);							//�������� �е�һ��, ������̶�
	}
	void Geo::clean()
	{
		vertexBuffer.destroy();
		indexBuffer.destroy();
	}
}