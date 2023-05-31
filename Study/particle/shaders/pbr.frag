#version 450
#include "part.pbr.frag"
//#extension GL_KHR_vulkan_glsl:enable
layout(set=1,binding=1) uniform sampler2D texSampler;

void main() 
{
    vec3 Lo = shade();outColor=vec4 (Lo.xyz,1.0);return;
    vec4 tex=texture(texSampler,inUV.xy);outColor=tex;return;
    outColor=tex*vec4(Lo,1.0);
}
