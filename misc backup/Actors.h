/* ���� tiny obj�ļ��ز��� */
#pragma once
#include "commonData.h"
#include <string>
#include <stdexcept>
#include <unordered_map>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "Geo.h"
#include "Scene.h"

using namespace tinyobj;
using namespace geos;
/* ���ڼ���һ�� ��ɫģ�� */
struct Actors
{
	const std::string model_path = "../models/robot/robot.obj";

	std::vector<Geo*> geos;		//һ��

	void loadModel(VulkanDevice* vulkanDevice,glm::mat4 model)
	{
		attrib_t attrib;
		std::vector<shape_t> shapes;
		std::vector<material_t> materials;
		std::string warn, err;
		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, model_path.c_str()))
		{
			throw std::runtime_error(warn + err);
		}

		for (const auto& shape : shapes)
		{
			Geo* item = new Geo();
			item->modelMatrix = model;
			geos.push_back(item);

			//std::unordered_map<Vertex, uint32_t> uniqueVertices{};//���ںϲ�����

			for (const auto& index : shape.mesh.indices)
			{
				Vertex vertex{};
				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};
				vertex.normal = {
					attrib.normals[3 * index.vertex_index + 0],
					attrib.normals[3 * index.vertex_index + 1],
					attrib.normals[3 * index.vertex_index + 2]
				};
				vertex.uv = {
					attrib.texcoords[2*index.texcoord_index+0],
					1.0f-attrib.texcoords[2*index.texcoord_index+1]
				};
				vertex.normal = glm::normalize(vertex.normal);
				vertex.color = { 1.0f,1.0f,1.0f,1.0f };
				//���� ��������?
				//if (0 == uniqueVertices.count(vertex)) 
				//{
				//	uniqueVertices[vertex] = static_cast<uint32_t>(item->vertices.size());
				//	item->vertices.push_back(vertex);
				//}
				//item->triangles.push_back(uniqueVertices[vertex]);
				item->vertices.push_back(vertex);
				item->triangles.push_back(item->triangles.size());//size() �Զ�����
			}
			//����ʱ��˳��
			for (int i = 0; i < item->triangles.size() / 3; i++) 
			{
				uint32_t a = item->triangles[i * 3 + 1];
				uint32_t b = item->triangles[i * 3 + 2];
				item->triangles[i * 3 + 1] = b;
				item->triangles[i * 3 + 2] = a;
			}
			//��������
			caculateTangent(&item->vertices);
		}
		for (int i = 0; i < geos.size(); i++)
		{
			geos[i]->prepareBuffer(vulkanDevice);
		}
	}
	void draw(VkCommandBuffer cmd,VkPipelineLayout piLayout) 
	{
		VkShaderStageFlags stageVGF = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		PerObjectData pod = { glm::mat4(1),{0,0,0,0} };
		uint32_t size = sizeof(PerObjectData);

		for (int i = 0; i < geos.size(); i++)
		{
			pod.model = geos[i]->modelMatrix;
			pod.texIndex = { i,0,0,0 };
			vkCmdPushConstants(cmd, piLayout, stageVGF, 0, size, &pod);
			geos[i]->drawGeo(cmd);
		}
	}
	void clean() 
	{
		for (int i = 0; i < geos.size(); i++)
		{
			geos[i]->clean();
		}
	}
	void caculateTangent(std::vector<Vertex>* vertices) 
	{
		/* ���߷�����  UV �仯���ķ���Ҳ����UV ���ݶȷ��� */
		for (uint32_t i = 0; i < vertices->size() / 3; i++)
		{
			Vertex p0 = (*vertices)[i * 3 + 0];
			Vertex p1 = (*vertices)[i * 3 + 1];
			Vertex p2 = (*vertices)[i * 3 + 2];
			glm::vec3 v1, v2;
			v1 = glm::vec3(p0.pos.x, p0.uv) - glm::vec3(p1.pos.x, p1.uv);
			v2 = glm::vec3(p0.pos.x, p0.uv) - glm::vec3(p2.pos.x, p2.uv);
			glm::vec3 n0 = glm::cross(v1, v2);//����

			v1 = glm::vec3(p0.pos.y, p0.uv) - glm::vec3(p1.pos.y, p1.uv);
			v2 = glm::vec3(p0.pos.y, p0.uv) - glm::vec3(p2.pos.y, p2.uv);
			glm::vec3 n1 = glm::cross(v1, v2);

			v1 = glm::vec3(p0.pos.z, p0.uv) - glm::vec3(p1.pos.z, p1.uv);
			v2 = glm::vec3(p0.pos.z, p0.uv) - glm::vec3(p2.pos.z, p2.uv);
			glm::vec3 n2 = glm::cross(v1, v2);

			p0.tangent = -glm::vec3(n1.x / n0.x, n1.y / n0.y, n1.z / n0.z);
			p1.tangent = -glm::vec3(n2.x / n1.x, n2.y / n1.y, n2.z / n1.z);
			p2.tangent = -glm::vec3(n0.x / n2.x, n0.y / n2.y, n0.z / n2.z);
			p0.tangent = glm::normalize(p0.tangent);
			p1.tangent = glm::normalize(p1.tangent);
			p2.tangent = glm::normalize(p2.tangent);

			(*vertices)[i * 3 + 0] = p0;
			(*vertices)[i * 3 + 1] = p1;
			(*vertices)[i * 3 + 2] = p2;
		}
	}
};
