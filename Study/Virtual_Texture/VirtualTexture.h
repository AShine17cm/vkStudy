#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <glm/glm.hpp>
//�ᱻ���ص������ڴ�� һ����������
struct VirtualTexturePage
{
	VkOffset3D offset;		
	VkExtent3D extent;		//����page ��һ��
	VkSparseImageMemoryBind imageMemoryBind;
	VkDeviceSize size;		//����page ��һ��
	uint32_t mipLevel;
	uint32_t layer;
	uint32_t index;			//�������� ȫ���б��е�˳��
	bool del;

	VirtualTexturePage();
	bool resident();		//�Ƿ��Ѿ����� ?
	bool allocate(VkDevice device, uint32_t memoryTypeIndex);
	bool release(VkDevice device);
};

//��������	�������е� Page, ��ЩPage�����Ѿ�����/δ������
struct  VirtualTexture
{
	VkDevice device;
	VkImage image;												//Image�� handle
	std::vector<VirtualTexturePage> pages;						//���� �Ѿ�����/δ���ص� page

	std::vector<VkSparseImageMemoryBind> sparseImageMemoryBinds;	//��ͨpage
	std::vector<VkSparseMemoryBind> opaqueMemoryBinds;				//mip tail

	VkSparseImageMemoryBindInfo imageMemoryBindInfo;				//��ͨpage ���
	VkSparseImageOpaqueMemoryBindInfo opaqueMemoryBindInfo;			//mip-tail ���
	VkBindSparseInfo bindSparseInfo;								//��ͨpage + mip-tail ��Ϣ

	uint32_t mipTailStart;											//mip-tail	��ʼ�ĵȼ���page/���� 128, ��Ӧ64��ʼ��mip
	VkSparseImageMemoryRequirements sparseImageMemoryRequirements;	//
	uint32_t memoryTypeIndex;										//

	VkSparseImageMemoryBind mipTailimageMemoryBind{};				//mip-tail ������ڴ��

	struct MipTailInfo
	{
		bool singleMipTail;
		bool alingedMipSize;
	}mipTailInfo;

	VirtualTexturePage* addPage(VkOffset3D offset, VkExtent3D extent, const VkDeviceSize size, const uint32_t mipLevel, uint32_t layer);
	void updateSparseBindInfo(std::vector<VirtualTexturePage>& bindingChangedPages, bool del = false);
	void destroy();
};


