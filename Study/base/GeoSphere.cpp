#include "Geo.h"


namespace geos
{
	/* clips是比值，不是绝对值 */
	GeoSphere::GeoSphere(float radius, vec2 clips, int uSegs, int vSegs)
	{
		clips.x = 1.0f - clips.x;
		clips.y = 1.0f - clips.y;
		float PI = 3.141592654f;// glm::pi<float>();
		/* sphere */
		float stepU = 1.0f / uSegs;
		float stepV = 1.0f / vSegs;
		float stepAng = PI * 2 / uSegs;
		float h0 = (-clips.y) * radius;
		float stepH = (clips.x + clips.y) * radius / vSegs;
		/* 环切面 */
		for (int v = 0; v <= vSegs; v++)
		{
			float h = h0 + stepH * v;
			float r = glm::sqrt(radius * radius - h * h);
			for (int u = 0; u <= uSegs; u++)
			{
				float ang = stepAng * u;
				float cos = glm::cos(ang);
				float sin = glm::sin(ang);
				Vertex pt{};
				vec3 pos = { cos * r,sin * r,h };//x,y,z
				vec3 nm = glm::normalize(pos);
				vec2 uv = { stepU * u,stepV * v };
				vec3 tmp = glm::cross({ 0,0,1 }, nm);
				vec3 tan = glm::cross(nm, tmp);
				tan = normalize(tan);

				pt.pos = pos;
				pt.normal = nm;
				pt.uv = uv;
				pt.tangent = tan;
				vertices.push_back(pt);
			}
		}

		/* 顶面 */
		for (int u = 0; u <= uSegs; u++) {
			float h = radius * clips.x;
			float r = glm::sqrt(radius * radius - h * h);
			for (int u = 0; u <= uSegs; u++)
			{
				float ang = stepAng * u;
				float cos = glm::cos(ang);
				float sin = glm::sin(ang);
				Vertex pt{};
				vec3 pos = { cos * r,sin * r,h };//x,y,z
				vec3 nm = { 0,0,1 };
				vec2 uv = { stepU * u,stepV * vSegs };
				vec3 tmp = glm::normalize(pos);
				vec3 tan = glm::cross(tmp, nm);
				tan = normalize(tan);

				pt.pos = pos;
				pt.normal = nm;
				pt.uv = uv;
				pt.tangent = tan;
				vertices.push_back(pt);
			}
		}

		/* 底面 */
		for (int u = 0; u <= uSegs; u++) {
			float h = radius * clips.x;
			float r = glm::sqrt(radius * radius - h * h);
			for (int u = 0; u <= uSegs; u++)
			{
				float ang = stepAng * u;
				float cos = glm::cos(ang);
				float sin = glm::sin(ang);
				Vertex pt{};
				vec3 pos = { cos * r,sin * r,h };//x,y,z
				vec3 nm = { 0,0,-1 };
				vec2 uv = { stepU * u,stepV * vSegs };
				vec3 tmp = glm::normalize(pos);
				vec3 tan = glm::cross(tmp, nm);
				tan = normalize(tan);

				pt.pos = pos;
				pt.normal = nm;
				pt.uv = uv;
				pt.tangent = tan;
				vertices.push_back(pt);
			}
		}
		/* 顶面中心 */
		Vertex pt{};
		pt.pos = { 0,0,radius * clips.x };
		pt.normal = { 0,0,1 };
		pt.tangent = { 0,1,0 };
		pt.uv = { 0,0 };
		vertices.push_back(pt);
		/* 底部中心 */
		pt.pos = { 0,0,-radius * clips.y };
		pt.normal = { 0,0,-1 };
		pt.tangent = { 0,1,0 };
		pt.uv = { 0,0 };
		vertices.push_back(pt);
		/* 三角面 */
		int idx_pt;
		for (int v = 0; v < vSegs; v++)
		{
			int offset = (uSegs + 1) * v;
			for (int u = 0; u < uSegs; u++)
			{
				idx_pt = offset + u;
				triangles.push_back(idx_pt);
				triangles.push_back(idx_pt + (uSegs + 1));
				triangles.push_back(idx_pt + 1);

				triangles.push_back(idx_pt + 1);
				triangles.push_back(idx_pt + (uSegs + 1));
				triangles.push_back(idx_pt + (uSegs + 1 + 1));
			}
		}
		/* 顶面 */
		int idx_Center = vertices.size() - 2;
		for (int u = 0; u < uSegs; u++) {
			idx_pt = (uSegs + 1) * (vSegs + 1) + u;;
			triangles.push_back(idx_pt + 1);
			triangles.push_back(idx_pt);
			triangles.push_back(idx_Center);
		}
		idx_Center = idx_Center + 1;
		for (int u = 0; u < uSegs; u++) {
			idx_pt = (uSegs + 1) * (vSegs + 2) + u;
			triangles.push_back(idx_pt);
			triangles.push_back(idx_pt + 1);
			triangles.push_back(idx_Center);
		}
	}

}