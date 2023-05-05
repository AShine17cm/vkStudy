#version 450

/* 将顶点转换到灯光空间 */
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inUv;
layout(location = 4) in vec3 inColor;

//layout(location=0) out vec4 outData;//测试用

layout(set=0,binding=0) uniform lightSpace
{
    mat4 vp;
}light;

layout(push_constant) uniform PushConstants
{
    mat4 model;
    vec4 tex;
} pushs;
out gl_PerVertex
{
    vec4 gl_Position;
};

void main() 
{
    gl_Position=light.vp*pushs.model*vec4(inPos,1);
    //outData=gl_Position;
}
