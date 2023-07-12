/* ÔÚÆÁÄ»ÉÏ»­Í¼ */
#version 450
//#extension GL_KHR_vulkan_glsl:enable

layout(set=0,binding=1) uniform sampler2D uiSampler;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;
void main() 
{
    outColor=texture(uiSampler,uv);
}
