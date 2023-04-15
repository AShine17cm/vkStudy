#include "materials.h"

namespace mg 
{
namespace materials 
{
    //����-д DescriptorSet
	void Material::writeDescriptorSets(VkDevice device,VkDescriptorPool descriptorPool) 
	{
        //���һ���򼸸� descriptorSet�Ǻ� Geo��ص�

        mg::descriptors::allocateDescriptorSet(
            &pShader->descLayout,
            1,
            descriptorPool, device,
            &descriptorSet);

        //д ÿһ��descriptorSet
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