#include "PerObjectData.h"

namespace geos 
{
	PbrMaterial::PbrMaterial()
	{
		baseColorFactor = { 1,1,1,1 };
		emissiveFactor = { 0,0,0,0 };
		diffuseFactor = { 1,1,1,1 };
		specularFactor = { 1,1,1,1 };

		baseColorTextureSet = 0;
		physicalDescriptorTextureSet = 0;
		normalTextureSet = 0;
		occlusionTextureSet = 0;
		emissiveTextureSet = -1;

		metallicFactor = 0.8f;
		roughnessFactor = 0.2f;
		alphaMask = 1.0f;
		alphaMaskCutoff = 1.0f;

		exposure = 4.5f;
		gamma = 2.2f;
		prefilteredCubeMipLevels = 10.0f;
		scaleIBLAmbient = 1.0f;

		//调试输入参数
		debugViewInputs = 0.0f;
		//调试 brdf 方程
		debugViewEquation = 0.0f;
	}
}