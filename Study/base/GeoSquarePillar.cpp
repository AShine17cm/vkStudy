#include "Geo.h"
namespace geos 
{
	/*
	���� �����ڵײ�
	*/
	GeoSquarePillar::GeoSquarePillar(vec2 baseSize, vec2 topSize, vec2 topCenter, vec2 height, vec2 uvPlane)
	{
		this->baseSize = baseSize;
		this->topSize = topSize;
		this->topCenter = topCenter;
		this->height = height;			//x ��׼��  yб��
		this->uvPlane = uvPlane;		//uv ͶӰ�ο���С
		/* �ײ���4����
		1-2
		0-3
		*/
		vec3 baseP0, baseP1, baseP2, baseP3;
		baseP0 = { -baseSize.x / 2,-baseSize.y / 2,0 };
		baseP1 = { -baseSize.x / 2,baseSize.y / 2,0 };
		baseP2 = { baseSize.x / 2,baseSize.y / 2,0 };
		baseP3 = { baseSize.x / 2,-baseSize.y / 2,0 };
		/* ������4����
		1-2
		0-3
		*/
		vec3 topP0, topP1, topP2, topP3;
		vec3 center= vec3(topCenter.x, topCenter.y, height.x);//height.x �Ǳ�׼�߶�
		topP0 = { -topSize.x / 2,-topSize.y / 2,0 };
		topP1 = { -topSize.x / 2,topSize.y / 2,0 };
		topP2 = { topSize.x / 2,topSize.y / 2,0 };
		topP3 = { topSize.x / 2,-topSize.y / 2,0 };
		topP0 += center;
		topP1 += center;
		topP2 += center;
		topP3 += center;
		/*  ��(base,top)ֱ���� ������ 2,3, ����б��Ч��
		1-2x
		0-3x
		*/
		vec3 lineBP = topP2 - baseP2;
		topP2 = baseP2 + (lineBP)*height.y;
		lineBP = topP3 - baseP3;
		topP3 = baseP3 + (lineBP)*height.y;

		Vertex v0, v1, v2, v3;
		/* 4������ */
		generateVertex(baseP0, baseP1, topP1, topP0, &v0, &v1, &v2, &v3, uvPlane);
		vertices.push_back(v0);
		vertices.push_back(v1);
		vertices.push_back(v2);
		vertices.push_back(v3);
		generateVertex(baseP1, baseP2, topP2, topP1,  &v0, &v1, &v2, &v3, uvPlane);
		vertices.push_back(v0);
		vertices.push_back(v1);
		vertices.push_back(v2);
		vertices.push_back(v3);
		generateVertex(baseP2, baseP3, topP3, topP2, &v0, &v1, &v2, &v3, uvPlane);
		vertices.push_back(v0);
		vertices.push_back(v1);
		vertices.push_back(v2);
		vertices.push_back(v3);
		generateVertex(baseP3, baseP0, topP0, topP3, &v0, &v1, &v2, &v3, uvPlane);
		vertices.push_back(v0);
		vertices.push_back(v1);
		vertices.push_back(v2);
		vertices.push_back(v3);
		/* ���� */
		generateVertex(topP0, topP1, topP2, topP3, &v0, &v1, &v2, &v3, uvPlane);
		vertices.push_back(v0);
		vertices.push_back(v1);
		vertices.push_back(v2);
		vertices.push_back(v3);

		//������ (û�е�)
		uint32_t idx_pt =0;
		for (uint32_t i = 0; i < 5; i++) 
		{
			idx_pt = i * 4;
			triangles.push_back(idx_pt);
			triangles.push_back(idx_pt + 1);
			triangles.push_back(idx_pt + 2);

			triangles.push_back(idx_pt);
			triangles.push_back(idx_pt + 2);
			triangles.push_back(idx_pt + 3);
		}
	}
}