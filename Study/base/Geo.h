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
几何体 顶点数据大小,着色器
Vulkan  Z-朝上  X-内-左  Y-内-右
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

		//顶点布局 vector不能放在函数内部
		static VkPipelineVertexInputStateCreateInfo getVertexInput(
			std::vector<VkVertexInputAttributeDescription>* attributes,
			std::vector<VkVertexInputBindingDescription>* bindings);
		static void getVertexInput_Instancing(
			std::vector<VkVertexInputAttributeDescription>* attributes,
			std::vector<VkVertexInputBindingDescription>* bindings,
			uint32_t instanceStride
		);
		/* 根据投影平面，计算UV */
		static void projectUV(vec3 p0, vec3 p1, vec3 p2, vec3 p3, vec2 uvPlane, vec2* uv0, vec2* uv1, vec2* uv2, vec2* uv3,bool isQuad=true)
		{
			//平面的中心 两个轴向
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

			//在投影平面上的 uv
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
		根据平面上的4个点 生成Normal,Tangent,UV
		*/
		static void generateVertex(vec3 p0, vec3 p1, vec3 p2, vec3 p3,Vertex* v0,Vertex* v1,Vertex* v2,Vertex* v3,vec2 uvPlane,bool isQuad=true) 
		{
			/* 一个平面公用一个 tangent */
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
				/* 四边形 */
				v2->tangent = tangent;
				v3->pos = p3;
				v3->normal = dirUp;
				v3->tangent = tangent;
			}
			else 
			{
				/* 三角形 */
				v2->tangent = tangent;
			}
			/* 计算UV */
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
	带两个切角的 立方体，每个面有5个点，3个三角形
	*/
	class GeoCube:public Geo
	{
	public:
		vec3 clipTop, clipBase;	//切角 
		float size;
		GeoCube(float size,vec3 clipA,vec3 clipB);
		/*
		5个点，p1Corner 是 corner, p0-p1 是标准size
		计算 法线，切线，UV
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
	方柱 中心在底部
	*/
	class GeoSquarePillar :public Geo 
	{

	public:
		vec2 baseSize;
		vec2 topSize;
		vec2 topCenter;
		vec2 height;		//x 标准高  y 侧边的伸缩比例
		vec2 uvPlane;		//uv 投影参考大小
		GeoSquarePillar(vec2 baseSize,vec2 topSize,vec2 topCenter,vec2 height,vec2 uvPlane);
	};
	/*
	圆柱体
	*/
	//class GeoCylinder:public Geo
	//{
	//public:
	//	float radius;
	//	float height;
	//	float clipAng;	//斜切角度
	//	int uSegs;		//环切面数量
	//	GeoCylinder(float radius, float height, float clipAng, int uSegs);

	//private:

	//};
	class  GeoSphere:public Geo
	{
	public:
		float radius;
		vec2 clips;		//上下切高度
		int uSegs, vSegs;	//U,V细分数量
		/* clips是比值，不是绝对值 */
		GeoSphere(float radius, vec2 clips, int uSegs,int vSegs);

	private:

	};

}
/* hash 函数需要放到 std 空间 */
namespace std {
	template<> struct hash<geos::Vertex> 
	{
		size_t operator()(geos::Vertex const& vertex) const 
		{
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.uv) << 1);
		}
	};
}