#version 450
#include "part.constants.shader"
/* 将顶点转换到灯光空间 */
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inUv;
layout(location = 4) in vec3 inColor;

struct Light{ mat4 mvp;vec4 vec;vec4 color;  };
layout(set=0,binding=0) uniform UniformObjectData
{
    mat4 proj;
    mat4 view;
    vec4 camera;
    ivec4 debugShadow;
    ivec4 debugDeferred;
    Light lights[LIGHT_COUNT];
}scene;

layout(push_constant) uniform PushConstants
{
    mat4 model;
    vec4 tex;
} pushs;

void main() 
{
    gl_Position=pushs.model*vec4(inPos,1);
}
