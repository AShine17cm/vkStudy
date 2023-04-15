#include "Geo.h"
namespace geos 
{
	GeoPlane::GeoPlane(float size, vec3 pos) 
	{
		this->size = size;
		this->pos = pos;

		uint colums = 3;
		vec3 dirA = { 1,0,0 };
		vec3 dirB = { 0,1,0 };
		vec3 normal = { 0,0,1 };
		vec4 color = { 1,1,1,1 };
		vec3 zero = -dirA * size * 0.5f - dirB * size * 0.5f;
		float stepUv = 1.0f/(colums - 1);
		float step = size / (colums-1);
		for (uint32_t a = 0; a < colums; a++)
		{
			for (uint32_t b = 0; b < colums; b++) 
			{
				vec3 pos = zero + dirA * (step * a) + dirB * (step * b);
				vec2 uv = {1.0f- stepUv * a,stepUv * b };
				vertices.push_back({ pos,normal,{0,0,0},uv,color });

				if (a >= 1 && b >= 1) 
				{
					triangles.push_back(a * colums + b - 1);
					triangles.push_back((a - 1) * colums + b - 1);
					triangles.push_back((a - 1) * colums + b);

					triangles.push_back(a * colums + b - 1);
					triangles.push_back((a - 1) * colums + b);
					triangles.push_back( a * colums + b);
				}
			}
		}
	}
}