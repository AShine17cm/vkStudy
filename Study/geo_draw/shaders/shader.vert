#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUv;
layout(location = 3) in vec3 inColor;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;

layout(push_constant) uniform PushConstants{
    mat4 model;
} pushs;

layout(location = 0) out vec3 fragColor;
layout(location=1)out vec2 uv;

void main() 
{
    gl_Position=mvp.proj * mvp.view * pushs.model * vec4(inPos,1.0);
    //gl_Position=mvp.proj * mvp.view * mvp.model * vec4(inPos,1.0);
    fragColor = vec3(inColor.r,inColor.g,inColor.b);
    uv=inUv;
}
