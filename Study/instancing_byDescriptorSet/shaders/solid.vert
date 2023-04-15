//场景用UBO, tint用UBO,    模型矩阵,可见性 用PushConstants
#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUv;
layout(location = 3) in vec3 inColor;

layout(set=0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 lightPos;
} scene;

layout(set=1,binding=0) uniform uboMat{
    vec4 tint;
} tint;

layout(push_constant) uniform PushConstants{
    mat4 model;
} pushs;

layout(location=0) out vec3 outNormal;
layout(location=1) out vec3 outLightVec;
layout(location=2) out vec3 outViewVec;
layout(location=3) out vec2 uv;

void main() 
{
    gl_Position=scene.proj * scene.view * pushs.model * vec4(inPos,1.0);

    outNormal=mat3(pushs.model)*inNormal;
    vec4 pos=pushs.model*vec4(inPos,1.0);
    outLightVec=scene.lightPos.xyz-pos.xyz;
    outViewVec=-pos.xyz;
    uv=inUv;
}
