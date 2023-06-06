#include "PerObjectData.h"

namespace geos 
{
	PbrMaterial::PbrMaterial()
	{
		baseColorFactor = { 1,1,1,1 };		//金属流
		emissiveFactor = { 1,1,1,1 };		//为 1
		diffuseFactor = { 1,1,1,1 };		//高光流
		specularFactor = { 0,0,0,1 };		//高光流
		//texture set 不是 0 就是 -1
		occlusionTextureSet = 0;
		emissiveTextureSet = -1;

		metallicFactor = 1.0f;	//有贴图 一般为 1
		roughnessFactor = 1.0f;	//有贴图 一般为 1
		alphaMask = 0.0f;
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