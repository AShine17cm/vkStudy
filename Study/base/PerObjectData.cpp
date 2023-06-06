#include "PerObjectData.h"

namespace geos 
{
	PbrMaterial::PbrMaterial()
	{
		baseColorFactor = { 1,1,1,1 };		//������
		emissiveFactor = { 1,1,1,1 };		//Ϊ 1
		diffuseFactor = { 1,1,1,1 };		//�߹���
		specularFactor = { 0,0,0,1 };		//�߹���
		//texture set ���� 0 ���� -1
		occlusionTextureSet = 0;
		emissiveTextureSet = -1;

		metallicFactor = 1.0f;	//����ͼ һ��Ϊ 1
		roughnessFactor = 1.0f;	//����ͼ һ��Ϊ 1
		alphaMask = 0.0f;
		alphaMaskCutoff = 1.0f;

		exposure = 4.5f;
		gamma = 2.2f;
		prefilteredCubeMipLevels = 10.0f;
		scaleIBLAmbient = 1.0f;

		//�����������
		debugViewInputs = 0.0f;
		//���� brdf ����
		debugViewEquation = 0.0f;
	}
}