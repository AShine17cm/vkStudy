/* ����Ļ�ϻ�ͼ */
#version 450

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;
layout(set=0,binding=1) uniform sampler2D uiSampler;

void main() 
{
    outColor=texture(uiSampler,uv);
}
