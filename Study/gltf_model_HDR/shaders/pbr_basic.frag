#version 450
//#extension GL_KHR_vulkan_glsl:enable
#include "part.constants.shader"
#include "part.shadow_layers.frag"
#include "part.pbr.frag"

//World-Space
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;      
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec2 inUV1;
layout(location = 4) in vec4 inColor;

layout(location = 0) out vec4 outColor;

//layout(set=0,binding=1)   uniform sampler2DArray shadowMap;

struct Light{ mat4 mvp;vec4 vec;vec4 color;};
layout(set=0,binding=0) uniform UniformObjectData
{
    mat4 proj;
    mat4 view;
    vec4 camera;
    ivec4 debug;
    Light lights[LIGHT_COUNT];
}scene;

layout(set=1,binding=0) uniform PbrBasic
{
    vec4 rgba;
    float roughness;
    float metallic;

    //高级参数
    float exposure;
	float gamma;
	float prefilteredCubeMipLevels;
	float scaleIBLAmbient;
	float debugViewInputs;
	float debugViewEquation;

}pbrBasic;


#define ALBEDO vec3(pbrBasic.rgba.rgb)

//法线分布 Normal Distribution
float D_GGX(float dotNH,float roughness)
{
    float alpha=roughness*roughness;
    float alpha2=alpha*alpha;
    float denom=dotNH*dotNH*(alpha2-1.0)+1.0;
    return (alpha2)/(PI*denom*denom);
//    float a2=roughness*roughness;
//    float denom=(dotNH*dotNH)*(a2-1.0)+1.0;
//    return a2/(PI*denom*denom);
}
//Geometric Shadowing function
float G_SchlicksmithGGX(float dotNL,float dotNV,float roughness)
{
    float r=(roughness+1.0);
    float k=(r*r)/8.0;
    float GL=dotNL/(dotNL*(1.0-k)+k);
    float GV=dotNV/(dotNV*(1.0-k)+k);
    return GL*GV;
}
//Fresnel function
vec3 F_Schlick(float cosTheta,float metallic)
{
    vec3 F0=mix(vec3(0.04),pbrBasic.rgba.rgb,metallic);
    vec3 F=F0+(1.0-F0)*pow(1.0-cosTheta,5.0);
    return F;
}
//Specular BRDF compose 双向反射-分布函数
vec3 BRDF(vec3 L,vec3 V,vec3 N,float metallic,float roughness,vec3 lightColor)
{
    vec3 H=normalize(V+L);
    float dotNV=clamp(dot(N,V),0.0,1.0);
    float dotNL=clamp(dot(N,L),0.0,1.0);
    //float dotLH=clamp(dot(L,H),0.0,1.0);
    float dotNH=clamp(dot(N,H),0.0,1.0);

    vec3 color=vec3(0.0);
    if(dotNL>0.0)
    {
        float rroughness=max(0.05,roughness);
        //D 法线分布函数
        float D=D_GGX(dotNH,roughness);//roughness
        //G 几何函数 自阴影 与 Mask
        float G=G_SchlicksmithGGX(dotNL,dotNV,rroughness);//r roughness
        vec3 F=F_Schlick(dotNV,metallic);

        vec3 spec=D*F*G/(4.0*dotNL*dotNV+0.001); //不加上0.001,容易出现黑色瑕疵
        vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);			
		//color += (kD * ALBEDO / PI + spec) * dotNL;
        color += (kD * ALBEDO / PI + spec) * dotNL*lightColor;
        //spec=vec3(G);
        //color+=spec*dotNL*lightColor;
    }
    return color;
}
/* 世界坐标 计算光照向量 */
vec3 shade( ) 
{
    vec3 N =normalize(inNormal);
    vec3 V=normalize(scene.camera.xyz-inPos);
    float roughness=pbrBasic.roughness;

    /* 采样阴影-多层 */
    float shadow[LIGHT_COUNT];
    for(int i=0;i<LIGHT_COUNT;i+=1)
    {
        vec4 coord=scene.lights[i].mvp* vec4(inPos,1.0);
        float layerVal=1.0;
        layerVal= filterPCF(coord,i);

        if(scene.debug[i]==0) layerVal=1.0;/* 3个光源的阴影,逐次展示，共同展示 */
        shadow[i]=layerVal;
    }
    
    vec3 Lo=vec3(0.0);
    for(int i=0;i<LIGHT_COUNT;i+=1)
    {
        vec4  vecL=scene.lights[i].vec;
        vec3  L=vecL.xyz;               //平行光(指向光源)
        L.y=-L.y;                       //灯光 翻转到 -Y轴, 和顶点的操作一致
        vec3 lightColor=scene.lights[i].color.rgb*scene.lights[i].color.a;
        Lo=Lo+BRDF(L,V,N,pbrBasic.metallic,roughness,lightColor)*shadow[i];
    }
    vec3 color=pbrBasic.rgba.rgb*0.02;
    color+=Lo;
    //color=pow(color,vec3(0.4545));
    return color;
}
void main() 
{
    vec3 Lo = shade();
    outColor=vec4(Lo,1.0);
}
