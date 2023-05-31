//#version 450
#include "part.constants.shader"
#include "part.shadow_layers.frag"

layout(location = 0) in vec3 inUV;
layout(location = 1) in vec3 inNormal;      //大部分时候 World-Space
layout(location = 2) in vec3 inPos;

layout(location = 0) out vec4 outColor;

//layout(set=0,binding=1)   uniform sampler2D shadowMap;
//layout(set=0,binding=1)   uniform sampler2DArray shadowMap;
//layout(set=1,binding=0)   uniform sampler2D texSampler;
//layout(set=1,binding=0)   uniform sampler3D texSampler;
//layout(set=1,binding=0)   uniform sampler2DArray samplerArray;
//layout(set=1,binding=0)   uniform samplerCube texSampler;

struct Light{ mat4 mvp;vec4 vec;vec4 color;};
layout(set=0,binding=0) uniform UniformObjectData
{
    mat4 proj;
    mat4 view;
    vec4 camera;
    ivec4 debug;
    Light lights[LIGHT_COUNT];
}scene;

layout(push_constant) uniform PushConstants
{
    mat4 model;
    vec4 tex;
} pushs;

/* 世界坐标 计算光照向量 */
vec4 shade( ) 
{
    vec3 N =normalize(inNormal);
    vec3 V=normalize(scene.camera.xyz-inPos);

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

    vec3 color=vec3(0);
    float spec=0;
    for(int i=0;i<LIGHT_COUNT;i+=1)
    {
        if(i==0)
        {
            vec4  vecL=scene.lights[i].vec;
            vec3  L=vecL.xyz;               //平行光(指向光源)
            float NdotL=max(0.0,dot(N,L));  //光照

            vec3 tmp=scene.lights[i].color.rgb*(NdotL*scene.lights[i].color.a);
            color=color+ tmp;//*shadow[i];
            //color=color+vec3(NdotL);

            vec3 R=reflect(-L,N);           //高光
            float NdotR=max(0.0,dot(R,V));
            spec+=pow(NdotR,16.0);//*shadow[i];
        }
    }

    vec3 diff=color;// vec3(spec);
    return vec4(diff,1.0);
}
