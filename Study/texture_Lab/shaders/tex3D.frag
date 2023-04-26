#version 450
#extension GL_KHR_vulkan_glsl:enable
/* 漫反射 wrap + 高光 */
/* VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT */
layout(push_constant) uniform PushConstants
{
    mat4 model;
    vec4 tex;
} pushs;

layout(set=1,binding=0) uniform sampler3D texSampler;

layout(location=0) in vec3 uv;
layout(location=1) in vec3 normal;
layout(location=2) in vec3 wldPos;
layout(location=3) in vec4 light;
layout(location=4) in vec3 camera;

layout(location = 0) out vec4 outColor;

void main() 
{
    /* 世界坐标 计算光照向量 */
    vec3 vecLit=light.xyz-wldPos;
    vec3 vecEye=camera-wldPos;
    vec3 wldNormal=mat3x3(pushs.model)*normal;
    float sqr=length(vecLit);
    vecLit= normalize(vecLit);
    vecEye=normalize(vecEye);
    wldNormal=normalize(wldNormal);
    /* 半角向量和高光 */
    vec3 hVec=normalize(vecLit+vecEye);
    float spec=max(0,dot(wldNormal,hVec));
    spec=pow(spec,20);
    /* 漫反射 wrap */
    float diff=0.02+max(0,(dot(vecLit,wldNormal)+0.4)/1.4)*light.w/sqr;

    vec4 d=texture(texSampler,uv);
    outColor=d*(diff+spec);
}
