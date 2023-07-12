#include "VirtualTexture.h"

VirtualTexturePage::VirtualTexturePage() {
	imageMemoryBind.memory = VK_NULL_HANDLE;
}

//纹理页 是否已经加载
bool VirtualTexturePage::resident() {
	return (imageMemoryBind.memory != VK_NULL_HANDLE);
}
//分配内存，所有Page 的内存大小，类型一致
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

	//内存绑定
	imageMemoryBind.subresource = subResource;
	imageMemoryBind.extent = extent;
	imageMemoryBind.offset = offset;
}
//释放 Page的内存
bool VirtualTexturePage::release(VkDevice device) {
	del = false;
	if (imageMemoryBind.memory != VK_NULL_HANDLE) {
		vkFreeMemory(device, imageMemoryBind.memory, nullptr);
		imageMemoryBind.memory = VK_NULL_HANDLE;
		return true;
	}
	return false;
}

//虚纹理，所有的Page 以及内存绑定信息
//在虚纹理中 添加一块 page, 包括 offset和大小,mipLevel,layer信息
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
	//把 变化的 page的绑定信息添加进来
	for (auto page : bindingChangedPages) {
		sparseImageMemoryBinds.push_back(page.imageMemoryBind);
		if (del) {
			sparseImageMemoryBinds[sparseImageMemoryBinds.size() - 1].memory = VK_NULL_HANDLE;
		}
	}
	//普通的page
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
	//清理 mip-tail
	if (mipTailimageMemoryBind.memory != VK_NULL_HANDLE)
	{
		vkFreeMemory(device, mipTailimageMemoryBind.memory, nullptr);
	}
}