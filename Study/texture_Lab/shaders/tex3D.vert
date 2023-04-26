//场景UBO,模型矩阵用PushConstants
#version 450

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

layout(push_constant) uniform PushConstants
{
    mat4 model;
    vec4 tex;
} pushs;

layout(location=0)out vec3 uv;
layout(location=1)out vec3 normal;
layout(location=2)out vec3 wldPos;
layout(location=3)out vec4 light;
layout(location=4)out vec3 camera;
void main() 
{
    normal=inNormal;
    wldPos=(pushs.model*vec4(inPos,1.0)).xyz;
    light=scene.light;
    camera=scene.camera;

    uv=normalize(inPos)*0.5+vec3(1,1,1)*0.5;
    uv.xy=normalize(uv.xy);
    uv.z=(wldPos.y+3.2)/6;//cube 队列的长度

    gl_Position=scene.proj * scene.view * vec4(wldPos,1.0);
}
