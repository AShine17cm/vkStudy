#include "Geo.h"

namespace geos
{
	/*
	带两个切角的 立方体，每个面有5个点，3个三角形
	对角	<top-1,base-3>   切面对 <top-0,base-2>
	clipTop 中的数值是比例大小
	*/
	GeoCube::GeoCube(float size, vec3 clipTop, vec3 clipBase)
	{
		this->size = size;
		this->clipTop = clipTop;
		this->clipBase = clipBase;
		float hSize = size * 0.5f;
		/*  底面
		1-2		v-21,v23
		0-3		  v-22
		*/
		vec3 baseP0 = { -hSize,-hSize,-hSize };
		vec3 baseP1 = { -hSize,hSize,-hSize };
		vec3 baseP2 = { hSize,hSize,-hSize };
		vec3 baseP3 = { hSize,-hSize,-hSize };
		/* 切面上的点 */
		vec3 baseC21 = baseP2; baseC21.x -= (clipBase.x * size);
		vec3 baseC23 = baseP2; baseC23.y -= (clipBase.y * size);
		vec3 baseC22 = baseP2; baseC22.z += (clipBase.z * size);

		/*  顶面
		1-2		v-01,v-03
		0-3		   v-00
		*/
		vec3 topP0 = { -hSize,-hSize,hSize };
		vec3 topP1 = { -hSize,hSize,hSize };
		vec3 topP2 = { hSize,hSize,hSize };
		vec3 topP3 = { hSize,-hSize,hSize };
		/* 切面上的点 */
		vec3 topC01 = topP0; topC01.y += (clipBase.x * size);
		vec3 topC03 = topP0; topC03.x += (clipBase.y * size);
		vec3 topC00 = topP0; topC00.z -= (clipBase.z * size);

		Vertex p0, p1, p2, p3, p4;
		/* 4个柱面 */
		generateVertex(baseP0, baseP1, topP1, topC01, topC00, &p0, &p1, &p2, &p3, &p4);
		vertices.push_back(p0);
		vertices.push_back(p1);
		vertices.push_back(p2);
		vertices.push_back(p3);
		vertices.push_back(p4);
		generateVertex(topP1, baseP1, baseC21, baseC22, topP2, &p0, &p1, &p2, &p3, &p4);
		vertices.push_back(p0);
		vertices.push_back(p1);
		vertices.push_back(p2);
		vertices.push_back(p3);
		vertices.push_back(p4);
		generateVertex(baseP3, topP3, topP2, baseC22, baseC23, &p0, &p1, &p2, &p3, &p4);
		vertices.push_back(p0);
		vertices.push_back(p1);
		vertices.push_back(p2);
		vertices.push_back(p3);
		vertices.push_back(p4);
		generateVertex(baseP3, baseP0, topC00, topC03, topP3, &p0, &p1, &p2, &p3, &p4);
		vertices.push_back(p0);
		vertices.push_back(p1);
		vertices.push_back(p2);
		vertices.push_back(p3);
		vertices.push_back(p4);
		/* 底面的点 */
		generateVertex(baseP1, baseP0, baseP3, baseC23, baseC21,  &p0, &p1, &p2, &p3, &p4);
		vertices.push_back(p0);
		vertices.push_back(p1);
		vertices.push_back(p2);
		vertices.push_back(p3);
		vertices.push_back(p4);
		/* 顶面上的点 */
		generateVertex(topP1, topP2, topP3, topC03, topC01, &p0, &p1, &p2, &p3, &p4);
		vertices.push_back(p0);
		vertices.push_back(p1);
		vertices.push_back(p2);
		vertices.push_back(p3);
		vertices.push_back(p4);
		/* 切面 */
		vec2 uvPlane = { size/2.0f,size/2.0f };
		Geo::generateVertex(baseC21, baseC23, baseC22, { 0,0,0 }, &p0, &p1, &p2, nullptr, uvPlane, false);
		vertices.push_back(p0);
		vertices.push_back(p1);
		vertices.push_back(p2);
		Geo::generateVertex(topC00, topC01, topC03, { 0,0,0 }, &p0, &p1, &p2, nullptr, uvPlane, false);
		vertices.push_back(p0);
		vertices.push_back(p1);
		vertices.push_back(p2);
		//五边形
		for (uint32_t s = 0; s < 6; s++)
		{
			uint32_t idx_pt = s * 5;
			//第2个点是 corner
			triangles.push_back(idx_pt);
			triangles.push_back(idx_pt + 1);
			triangles.push_back(idx_pt + 4);

			triangles.push_back(idx_pt + 4);
			triangles.push_back(idx_pt + 1);
			triangles.push_back(idx_pt + 3);

			triangles.push_back(idx_pt + 3);
			triangles.push_back(idx_pt + 1);
			triangles.push_back(idx_pt + 2);
		}
		uint32_t idx_pt = 6 * 5;
		triangles.push_back(idx_pt);
		triangles.push_back(idx_pt + 1);
		triangles.push_back(idx_pt + 2);

		triangles.push_back(idx_pt + 3);
		triangles.push_back(idx_pt + 4);
		triangles.push_back(idx_pt + 5);
	}



}