//场景UBO,模型矩阵用PushConstants
//#version 450
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inUv;
layout(location = 4) in vec3 inColor;

layout(location=0)out vec3 uv;
layout(location=1)out vec3 normal;
layout(location=2)out vec3 posWS;
layout(location=3)out vec4 light;
layout(location=4)out vec3 camera;
layout(location=5)out vec4 shadowCoord;//用于采样阴影贴图

/* 场景矩阵 */
layout(set=0, binding = 0) uniform UniformBufferObject 
{
    mat4 view;
    mat4 proj;
    mat4 lightSpace;// 将 world-Space 坐标转换到 light-Space
    vec4 light;
    vec3 camera;
} scene;

layout(push_constant) uniform PushConstants
{
    mat4 model;
    vec4 tex;
} pushs;

const mat4 biasMatrix=mat4(
    0.5,0.0,0.0,0.0,
    0.0,0.5,0.0,0.0,
    0.0,0.0,1.0,0.0,
    0.5,0.5,0.0,1.0);



