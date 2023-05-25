//单光源的阴影贴图
//#version 450
#include "part.constants.shader"
layout(set= 0,binding= 1)   uniform sampler2D shadowMap;

float textureProj(vec4 shadowCoord,vec2 offset)//offset 用于Soft-Shadow
{
    vec4 coord=shadowCoord/shadowCoord.w;       //此步骤是必须的，不然coord会出现奇怪的缩放
    float shadow=1.0;
    float dist=texture(shadowMap,coord.xy+offset).r;
    if(coord.z>-1.0&&coord.z<1.0)
    {
        if(coord.w>0.0&&dist<coord.z)
        {
            shadow=AMBIENT;
        }
    }
    return shadow;
}
float filterPCF(vec4 sc)
{
	ivec2 texDim = textureSize(shadowMap, 0).xy;
	float scale = 1.5;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 1;
	
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			shadowFactor += textureProj(sc, vec2(dx*x, dy*y));
			count++;
		}
	}
	return shadowFactor / count;
}
