#include "VirtualTexture.h"

VirtualTexturePage::VirtualTexturePage() {
	imageMemoryBind.memory = VK_NULL_HANDLE;
}

//����ҳ �Ƿ��Ѿ�����
bool VirtualTexturePage::resident() {
	return (imageMemoryBind.memory != VK_NULL_HANDLE);
}
//�����ڴ棬����Page ���ڴ��С������һ��
bool VirtualTexturePage::allocate(VkDevice device, uint32_t memoryTypeIndex) {
	if (imageMemoryBind.memory != VK_NULL_HANDLE) {
		return false;
	}
	imageMemoryBind = {};
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = size;
	allocInfo.memoryTypeIndex = memoryTypeIndex;
	vkAllocateMemory(device, &allocInfo, nullptr, &imageMemoryBind.memory);

	VkImageSubresource subResource{};
	subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subResource.mipLevel = mipLevel;
	subResource.arrayLayer = layer;

	//�ڴ��
	imageMemoryBind.subresource = subResource;
	imageMemoryBind.extent = extent;
	imageMemoryBind.offset = offset;
}
//�ͷ� Page���ڴ�
bool VirtualTexturePage::release(VkDevice device) {
	del = false;
	if (imageMemoryBind.memory != VK_NULL_HANDLE) {
		vkFreeMemory(device, imageMemoryBind.memory, nullptr);
		imageMemoryBind.memory = VK_NULL_HANDLE;
		return true;
	}
	return false;
}

//���������е�Page �Լ��ڴ����Ϣ
//���������� ���һ�� page, ���� offset�ʹ�С,mipLevel,layer��Ϣ
VirtualTexturePage* VirtualTexture::addPage(VkOffset3D offset, VkExtent3D extent, const VkDeviceSize size, const uint32_t mipLevel, uint32_t layer)
{
	VirtualTexturePage newPage{};
	newPage.offset = offset;
	newPage.extent = extent;
	newPage.size = size;
	newPage.mipLevel = mipLevel;
	newPage.layer = layer;
	newPage.index = static_cast<uint32_t>(pages.size());
	newPage.imageMemoryBind = {};
	newPage.imageMemoryBind.offset = offset;
	newPage.imageMemoryBind.extent = extent;
	newPage.del = false;
	pages.push_back(newPage);
	return &pages.back();
}

// Call before sparse binding to update memory bind list etc.
void VirtualTexture::updateSparseBindInfo(std::vector<VirtualTexturePage>& bindingChangedPages, bool del)
{
	sparseImageMemoryBinds.clear();
	//�� �仯�� page�İ���Ϣ��ӽ���
	for (auto page : bindingChangedPages) {
		sparseImageMemoryBinds.push_back(page.imageMemoryBind);
		if (del) {
			sparseImageMemoryBinds[sparseImageMemoryBinds.size() - 1].memory = VK_NULL_HANDLE;
		}
	}
	//��ͨ��page
	imageMemoryBindInfo = {};
	imageMemoryBindInfo.image = image;
	imageMemoryBindInfo.bindCount = static_cast<uint32_t>(sparseImageMemoryBinds.size());
	imageMemoryBindInfo.pBinds = sparseImageMemoryBinds.data();


	//mip-tail
	opaqueMemoryBindInfo.image = image;
	opaqueMemoryBindInfo.bindCount = static_cast<uint32_t>(opaqueMemoryBinds.size());
	opaqueMemoryBindInfo.pBinds = opaqueMemoryBinds.data();

	bindSparseInfo={};
	bindSparseInfo.sType = VK_STRUCTURE_TYPE_BIND_SPARSE_INFO;
	bindSparseInfo.imageBindCount = (imageMemoryBindInfo.bindCount > 0) ? 1 : 0;
	bindSparseInfo.pImageBinds = &imageMemoryBindInfo;

	bindSparseInfo.imageOpaqueBindCount = (opaqueMemoryBindInfo.bindCount > 0) ? 1 : 0;
	bindSparseInfo.pImageOpaqueBinds = &opaqueMemoryBindInfo;
}

void VirtualTexture::destroy() 
{
	for (auto page : pages) {
		page.release(device);
	}
	for (auto bind : opaqueMemoryBinds) {
		vkFreeMemory(device, bind.memory, nullptr);
	}
	//���� mip-tail
	if (mipTailimageMemoryBind.memory != VK_NULL_HANDLE)
	{
		vkFreeMemory(device, mipTailimageMemoryBind.memory, nullptr);
	}
}