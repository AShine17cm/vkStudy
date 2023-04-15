//使用 descriptorSet 输入instance数据
#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUv;
layout(location = 3) in vec3 inColor;

struct InstanceData
{
    mat4 model;
    vec4 texIndex;
};//8*(16+4)

layout(set=0, binding = 0) uniform uboScene {
    mat4 view;
    mat4 proj;
    vec3 lightPos;
} scene;

layout(set=1,binding=0) uniform uboInstance {
    InstanceData instances[1024*64/8/20];   //64*1024
}instances;

layout(location=0) out vec3 outNormal;
layout(location=1) out vec3 outLightVec;
layout(location=2) out vec3 outViewVec;
layout(location=3) out vec3 outUV;

void main() 
{
    mat4 model=instances.instances[gl_InstanceIndex].model;
    float texIndex=instances.instances[gl_InstanceIndex].texIndex.x;

    vec4 wldPos=model*vec4(inPos,1.0);
    gl_Position=scene.proj * scene.view * vec4(wldPos.xyz,1.0);

    outNormal=mat3(model)*inNormal;
    outLightVec=scene.lightPos-wldPos.xyz;
    outViewVec=-mat3(scene.view)*wldPos.xyz;
    outUV=vec3(inUv,texIndex);
}
