#pragma once
#include <vector>
#include "glm.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "gtx/hash.hpp"
#include "vulkan/vulkan.h"
#include "VulkanDevice.h"
#include "Buffer.h"
using namespace glm;
/*
������ �������ݴ�С,��ɫ��
Vulkan  Z-����  X-��-��  Y-��-��
*/
namespace geos 
{
	struct Vertex
	{
		vec3 pos;
		vec3 normal;
		vec3 tangent;
		vec2 uv;
		vec4 color;

		bool operator==(const Vertex& other)const
		{
			return
				pos == other.pos &&
				normal == other.normal &&
				tangent == other.tangent &&
				uv == other.uv &&
				color == other.color;
		}
	};

	class Geo
	{
	public:
		std::vector<Vertex> vertices;
		std::vector<uint32_t> triangles;
		mat4 modelMatrix = glm::mat4(1.0f);
		vec3 pos{ 0, 0, 0 };
		vec3 scale{ 1,1,1 };
		vec3 rotate{ 0,0,0 };
		float size = 1.0f;
		const char* name;
		VkIndexType indexType = VK_INDEX_TYPE_UINT32;

		mg::Buffer vertexBuffer;
		mg::Buffer indexBuffer;

		//���㲼�� vector���ܷ��ں����ڲ�
		static VkPipelineVertexInputStateCreateInfo getVertexInput(
			std::vector<VkVertexInputAttributeDescription>* attributes,
			std::vector<VkVertexInputBindingDescription>* bindings);
		static void getVertexInput_Instancing(
			std::vector<VkVertexInputAttributeDescription>* attributes,
			std::vector<VkVertexInputBindingDescription>* bindings,
			uint32_t instanceStride
		);
		/* ����ͶӰƽ�棬����UV */
		static void projectUV(vec3 p0, vec3 p1, vec3 p2, vec3 p3, vec2 uvPlane, vec2* uv0, vec2* uv1, vec2* uv2, vec2* uv3,bool isQuad=true)
		{
			//ƽ������� ��������
			vec3 center = (p0 + p1 + p2 + p3) /4.0f;
			if (!isQuad)
			{
				center = (p0 + p1 + p3) / 3.0f;
			}
			vec3 dirA = glm::normalize(p0 - center);
			vec3 dirB = glm::normalize(p1 - center);
			vec3 dirUp = glm::cross(dirA, dirB);
			dirB = glm::cross(dirUp, dirA);
			dirB = glm::normalize(dirB);

			//��ͶӰƽ���ϵ� uv
			float u = glm::dot(dirA, (p0 - center))/uvPlane.x;
			float v = glm::dot(dirB, (p0 - center))/uvPlane.y;
			*uv0 = { u + 0.5f,v + 0.5f };
			u = glm::dot(dirA, (p1 - center))/uvPlane.x;
			v = glm::dot(dirB, (p1 - center))/uvPlane.y;
			*uv1 = { u + 0.5f,v + 0.5f };
			u = glm::dot(dirA, (p2 - center))/uvPlane.x;
			v = glm::dot(dirB, (p2 - center))/uvPlane.y;
			*uv2 = { u + 0.5f,v + 0.5f };
			if(isQuad)
			{
				u = glm::dot(dirA, (p3 - center))/uvPlane.x;
				v = glm::dot(dirB, (p3 - center))/uvPlane.y;
				*uv3 = { u + 0.5f,v + 0.5f };
			}
		}
		/*
		����ƽ���ϵ�4���� ����Normal,Tangent,UV
		*/
		static void generateVertex(vec3 p0, vec3 p1, vec3 p2, vec3 p3,Vertex* v0,Vertex* v1,Vertex* v2,Vertex* v3,vec2 uvPlane,bool isQuad=true) 
		{
			/* һ��ƽ�湫��һ�� tangent */
			vec3 tangent = glm::normalize(p1 - p0);
			vec3 dirA = glm::normalize(p1 - p0);
			vec3 dirB = glm::normalize(p2 - p1);
			vec3 dirUp = glm::cross(dirB, dirA);
			dirUp = glm::normalize(dirUp);
			v0->pos = p0;
			v0->normal = dirUp;
			v0->tangent = tangent;
			v1->pos = p1;
			v1->normal = dirUp;
			v1->tangent = tangent;
			v2->pos = p2;
			v2->normal = dirUp;
			if (isQuad) 
			{
				/* �ı��� */
				v2->tangent = tangent;
				v3->pos = p3;
				v3->normal = dirUp;
				v3->tangent = tangent;
			}
			else 
			{
				/* ������ */
				v2->tangent = tangent;
			}
			/* ����UV */
			vec2 uv0, uv1, uv2, uv3;
			projectUV(p0, p1, p2, p3, uvPlane, &uv0, &uv1, &uv2, &uv3,isQuad);
			v0->uv = uv0;
			v1->uv = uv1;
			v2->uv = uv2;
			if (isQuad) 
			{
				v3->uv = uv3;
			}
		}
		virtual void prepareBuffer(mg::VulkanDevice* vulkanDevice);
		virtual void drawGeo(VkCommandBuffer cmd,int instanceCount=1);
		virtual void clean( );
	};
	/*
	�������нǵ� �����壬ÿ������5���㣬3��������
	*/
	class GeoCube:public Geo
	{
	public:
		vec3 clipTop, clipBase;	//�н� 
		float size;
		GeoCube(float size,vec3 clipA,vec3 clipB);
		/*
		5���㣬p1Corner �� corner, p0-p1 �Ǳ�׼size
		���� ���ߣ����ߣ�UV
		*/
		static void generateVertex(
			vec3 p0Axis, vec3 p1Corner, vec3 p2Axis, vec3 p3Clip0, vec3 p3Clip1, 
			Vertex* v0, Vertex* v1, Vertex* v2, Vertex* v3, Vertex* v4) 
		{
			vec3 dirU = glm::normalize(p0Axis - p1Corner);
			vec3 dirV = glm::normalize(p2Axis - p1Corner);
			vec3 normal = glm::normalize(glm::cross(dirU, dirV));
			vec3 tangent = dirU;

			float size = glm::length(p0Axis - p1Corner);
			float u, v;
			v0->pos = p0Axis;
			v0->normal = normal;
			v0->tangent = tangent;
			v1->pos = p1Corner;
			v1->normal = normal;
			v1->tangent = tangent;
			v2->pos = p2Axis;
			v2->normal = normal;
			v2->tangent = tangent;
			v3->pos = p3Clip0;
			v3->normal = normal;
			v3->tangent = tangent;
			v4->pos = p3Clip1;
			v4->normal = normal;
			v4->tangent = tangent;
			u = glm::dot(dirU, p0Axis - p1Corner)/size;
			v = glm::dot(dirV, p0Axis - p1Corner)/size;
			v0->uv = { u,v };
			v1->uv = { 0,0 };
			u = glm::dot(dirU, p2Axis - p1Corner)/size;
			v = glm::dot(dirV, p2Axis - p1Corner)/size;
			v2->uv = { u,v };
			u = glm::dot(dirU, p3Clip0 - p1Corner)/size;
			v = glm::dot(dirV, p3Clip0 - p1Corner)/size;
			v3->uv = { u,v };
			u = glm::dot(dirU, p3Clip1 - p1Corner)/size;
			v = glm::dot(dirV, p3Clip1 - p1Corner)/size;
			v4->uv = { u,v };
		}
	};
	class GeoPlane:public Geo
	{
	public:
		GeoPlane(float size, vec3 pos, vec3 normal = {0,0,1});
	};

	/*
	���� �����ڵײ�
	*/
	class GeoSquarePillar :public Geo 
	{

	public:
		vec2 baseSize;
		vec2 topSize;
		vec2 topCenter;
		vec2 height;		//x ��׼��  y ��ߵ���������
		vec2 uvPlane;		//uv ͶӰ�ο���С
		GeoSquarePillar(vec2 baseSize,vec2 topSize,vec2 topCenter,vec2 height,vec2 uvPlane);
	};
	/*
	Բ����
	*/
	//class GeoCylinder:public Geo
	//{
	//public:
	//	float radius;
	//	float height;
	//	float clipAng;	//б�нǶ�
	//	int uSegs;		//����������
	//	GeoCylinder(float radius, float height, float clipAng, int uSegs);

	//private:

	//};
	class  GeoSphere:public Geo
	{
	public:
		float radius;
		vec2 clips;		//�����и߶�
		int uSegs, vSegs;	//U,Vϸ������
		/* clips�Ǳ�ֵ�����Ǿ���ֵ */
		GeoSphere(float radius, vec2 clips, int uSegs,int vSegs);

	private:

	};

}
/* hash ������Ҫ�ŵ� std �ռ� */
namespace std {
	template<> struct hash<geos::Vertex> 
	{
		size_t operator()(geos::Vertex const& vertex) const 
		{
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.uv) << 1);
		}
	};
}