#version 450
#extension GL_KHR_vulkan_glsl:enable
/* ������ wrap + �߹� */
/* VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT */
layout(push_constant) uniform PushConstants
{
    mat4 model;
    vec4 tex;
} pushs;

layout(set=1,binding=0) uniform samplerCube texSampler;

layout(location=0) in vec2 uv;
layout(location=1) in vec3 normal;
layout(location=2) in vec3 wldPos;
layout(location=3) in vec4 light;
layout(location=4) in vec3 camera;

layout(location = 0) out vec4 outColor;

void main() 
{
    /* �������� ����������� */
    vec3 vecLit=light.xyz-wldPos;
    vec3 vecEye=camera-wldPos;
    vec3 normalWS=mat3x3(pushs.model)*normal;
    float sqr=length(vecLit);
    vecLit= normalize(vecLit);
    vecEye=normalize(vecEye);
    normalWS=normalize(normalWS);

    //relfect(in,normal)
    mat4 invModel=inverse(pushs.model);
    vec3 cI=normalize(wldPos);
    vec3 cR=reflect(cI,normalWS);
    //cR=mat3x3(invModel)*cR;
    //cR.xy*=-1.0;

    vec4 tex=texture(texSampler,cR,0);

    /* ��������͸߹� */
    vec3 hVec=normalize(vecLit+vecEye);
    float spec=max(0,dot(normalWS,hVec));
    spec=pow(spec,20);
    /* ������ wrap */
    float diff=0.02+max(0,(dot(vecLit,normalWS)+0.4)/1.4)*light.w/sqr;

    outColor=tex*(diff+spec);
}
