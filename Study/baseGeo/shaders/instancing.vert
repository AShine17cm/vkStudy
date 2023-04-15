//场景UBO,模型矩阵用PushConstants
#version 450
#extension GL_KHR_vulkan_glsl:enable
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inUv;
layout(location = 4) in vec3 inColor;

/* 场景矩阵 */
layout(set=0, binding = 0) uniform UniformBufferObject 
{
    mat4 view;
    mat4 proj;
    vec4 light;
    vec3 camera;
} scene;

/*这个 push_constant 没用,只是为了保持 layout的一致*/
layout(push_constant) uniform PushConstants
{
    mat4 model;
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

layout(location=0)out vec2 uv;
layout(location=1)out vec3 normal;
layout(location=2)out vec3 wldPos;
layout(location=3)out vec4 light;
layout(location=4)out vec3 camera;
layout(location=5)out float texIndex;
void main() 
{
    uv=inUv;
    mat4 model=instances.instances[gl_InstanceIndex].model;
    texIndex=instances.instances[gl_InstanceIndex].texIndex.x;
    normal=mat3(model)*inNormal;

    wldPos=(model*vec4(inPos,1.0)).xyz;
    light=scene.light;
    camera=scene.camera;

    gl_Position=scene.proj * scene.view * vec4(wldPos,1.0);
}
