#version 450
//#extension GL_KHR_vulkan_glsl:enable
/* 漫反射 wrap + 高光 */
/* VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT */
layout(push_constant) uniform PushConstants/* 未使用 */
{
    mat4 model;
    vec4 tex;
} pushs;

/*------ 实例化 数据 ------*/
struct InstanceData
{
    mat4 model;
    vec4 texIndex;
};//8*(16+4)
layout(set=1,binding=0) uniform uboInstance {
    InstanceData instances[1024*64/8/20];   //64*1024
}instances;
/*-------------------------*/

layout(set=2,binding=0) uniform sampler2D texSampler;

layout(location=0) in vec2 uv;
layout(location=1) in vec3 normal;
layout(location=2) in vec3 wldPos;
layout(location=3) in vec4 light;
layout(location=4) in vec3 camera;
layout(location=5) in float texIndex;

layout(location = 0) out vec4 outColor;

void main() 
{
    /* 世界坐标 计算光照向量 */
    vec3 vecLit=light.xyz-wldPos;
    vec3 vecEye=camera-wldPos;
    //mat4 model=instances.instances[gl_InstanceIndex].model;
    //vec3 wldNormal=mat3x3(model)*normal;
    vec3 wldNormal=normal;

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

    vec4 tex=texture(texSampler,uv);
    outColor=tex*(diff+spec);
}
