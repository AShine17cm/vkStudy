#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <glm/glm.hpp>
//会被加载到物理内存的 一块虚拟纹理
struct VirtualTexturePage
{
	VkOffset3D offset;		
	VkExtent3D extent;		//所有page 都一样
	VkSparseImageMemoryBind imageMemoryBind;
	VkDeviceSize size;		//所有page 都一样
	uint32_t mipLevel;
	uint32_t layer;
	uint32_t index;			//在虚纹理 全局列表中的顺序
	bool del;

	VirtualTexturePage();
	bool resident();		//是否已经加载 ?
	bool allocate(VkDevice device, uint32_t memoryTypeIndex);
	bool release(VkDevice device);
};

//虚拟纹理	包含所有的 Page, 这些Page可能已经加载/未被加载
struct  VirtualTexture
{
	VkDevice device;
	VkImage image;												//Image的 handle
	std::vector<VirtualTexturePage> pages;						//所有 已经加载/未加载的 page

	std::vector<VkSparseImageMemoryBind> sparseImageMemoryBinds;	//普通page
	std::vector<VkSparseMemoryBind> opaqueMemoryBinds;				//mip tail

	VkSparseImageMemoryBindInfo imageMemoryBindInfo;				//普通page 相关
	VkSparseImageOpaqueMemoryBindInfo opaqueMemoryBindInfo;			//mip-tail 相关
	VkBindSparseInfo bindSparseInfo;								//普通page + mip-tail 信息

	uint32_t mipTailStart;											//mip-tail	开始的等级，page/粒度 128, 对应64开始的mip
	VkSparseImageMemoryRequirements sparseImageMemoryRequirements;	//
	uint32_t memoryTypeIndex;										//

	VkSparseImageMemoryBind mipTailimageMemoryBind{};				//mip-tail 所需的内存绑定

	struct MipTailInfo
	{
		bool singleMipTail;
		bool alingedMipSize;
	}mipTailInfo;

	VirtualTexturePage* addPage(VkOffset3D offset, VkExtent3D extent, const VkDeviceSize size, const uint32_t mipLevel, uint32_t layer);
	void updateSparseBindInfo(std::vector<VirtualTexturePage>& bindingChangedPages, bool del = false);
	void destroy();
};


