//#version 450
#include "part.constants.shader"
#include "part.shadow_layers.frag"

layout(location = 0) in vec3 inUV;
layout(location = 1) in vec3 inNormal;      //大部分时候 World-Space
layout(location = 2) in vec3 inPos;

layout(location = 0) out vec4 outColor;

//layout(set=0,binding=1)   uniform sampler2D shadowMap;
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
}pbrBasic;

layout(push_constant) uniform PushConstants
{
    mat4 model;
    vec4 tex;
} pushs;

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

        vec3 spec=D*F*G/(4.0*dotNL*dotNV);
        spec=vec3(G);
        color+=spec*dotNL*lightColor;
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
        #ifdef SHADOW_PCF
            layerVal= filterPCF(coord,i);
        #else
            layerVal= textureProj(coord,i,vec2(0));
        #endif
        if(scene.debug[i]==0) layerVal=1.0;/* 3个光源的阴影,逐次展示，共同展示 */
        shadow[i]=layerVal;
    }

    vec3 Lo=vec3(0.0);
    for(int i=0;i<1;i+=1)
    {
        vec4  vecL=scene.lights[i].vec;
        vec3  L=vecL.xyz;               //平行光(指向光源)
        //L=normalize(scene.lights[i].vec.xyz-inPos);
        //L.xz=-L.xz;
        vec3 lightColor=scene.lights[i].color.rgb*scene.lights[i].color.a;
        Lo=Lo+BRDF(L,V,N,pbrBasic.metallic,roughness,lightColor)*shadow[i];
    }
    vec3 color=pbrBasic.rgba.rgb*0.02;
    color+=Lo;
    color=pow(color,vec3(0.4545));
    color=Lo;
    return color;
}
