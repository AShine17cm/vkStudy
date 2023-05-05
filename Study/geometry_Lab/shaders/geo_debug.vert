#version 450
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inUv;
layout(location = 4) in vec3 inColor;

layout(location=0)out vec3 normal;

void main() 
{
    /* �����Ϊ Local-Space  ����Geometry-Shader ���� */
    normal=inNormal;
    gl_Position=vec4(inPos,1.0);
}
