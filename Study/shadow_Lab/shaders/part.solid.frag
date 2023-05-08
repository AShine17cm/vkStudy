//#version 450
/* 漫反射 wrap + 高光 */
layout(location = 0) in vec3 inUV;
layout(location = 1) in vec3 inNormal;      //大部分时候 World-Space
layout(location = 2) in vec3 inPos;
layout(location = 3) in vec4 inLight;
layout(location = 4) in vec3 camera;
layout(location = 5) in vec4 shadowCoord;//阴影贴图中的坐标

layout(location = 0) out vec4 outColor;

layout(set= 0,binding= 1)   uniform sampler2D shadowMap;//和 scene相关的ubo在同一个 descriptorSet中，方便在cmd中绑定
//layout(set=1,binding=0)   uniform sampler2D texSampler;
//layout(set=1,binding=0)   uniform sampler3D texSampler;
//layout(set=1,binding=0)   uniform sampler2DArray samplerArray;
//layout(set=1,binding=0)   uniform samplerCube texSampler;

layout(push_constant) uniform PushConstants
{
    mat4 model;
    vec4 tex;
} pushs;

/*阴影相关*/
#define ambient 0.1
//可以在 rasterization开启depthBiasEnable, 同时加入到dynamicState,然后在 commandBuffer动态设置偏移
//#define zbias 0.000001  
float textureProj(vec4 shadowCoord,vec2 offset)//offset 用于Soft-Shadow
{
    vec4 coord=shadowCoord/shadowCoord.w;       //此步骤是必须的，不然coord会出现奇怪的缩放
    float shadow=1.0;
    float dist=texture(shadowMap,coord.xy+offset).r;
    if(coord.z>-1.0&&coord.z<1.0)
    {
        if(coord.w>0.0&&dist<coord.z)
        {
            shadow=ambient;
            //shadow=dist;
        }
    }
    return shadow;
}
float filterPCF(vec4 sc)
{
	ivec2 texDim = textureSize(shadowMap, 0);
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
/* 世界坐标 计算光照向量 */
void shade(out float diff,out float spec) 
{
    vec3 vecLit = inLight.xyz;
    vec3 vecEye = camera - inPos;
    vec3 wldNormal =normalize( inNormal);
    vecEye = normalize(vecEye);
    /* 半角向量和高光 */
    vec3 hVec = normalize(vecLit + vecEye);
    spec = max(0, dot(wldNormal, hVec));
    spec = pow(spec, 4);

    /* 漫反射 wrap */
    diff = 0.02 + max(0, (dot(vecLit, wldNormal) + 0.4)) / 1.4 * inLight.w;
    //diff=max(0, dot(vecLit, wldNormal));
}
