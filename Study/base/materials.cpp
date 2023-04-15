#include "materials.h"

namespace mg 
{
namespace materials 
{
    //分配-写 DescriptorSet
	void Material::writeDescriptorSets(VkDevice device,VkDescriptorPool descriptorPool) 
	{
        //最后一个或几个 descriptorSet是和 Geo相关的

        mg::descriptors::allocateDescriptorSet(
            &pShader->descLayout,
            1,
            descriptorPool, device,
            &descriptorSet);

        //写 每一个descriptorSet
        mg::descriptors::writeDescriptorSet(
            pShader->typeSet.data(),
            writeInfoSet.data(),
            writeInfoSet.size(),
            descriptorSet,
            device);

	}
	void Material::beginMaterial(VkCommandBuffer cmd)
	{
		//vkCmdBindPipeline(
        //  cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, 
        //  pShader->pipeline);

		vkCmdBindDescriptorSets(
			cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
			pShader->layout,
			0,
            1,
			&descriptorSet,
			0, nullptr);
	}
}
}