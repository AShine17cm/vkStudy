#version 450
#include "part.pbr.frag"
//#extension GL_KHR_vulkan_glsl:enable
layout(set=1,binding=0) uniform sampler2D texSampler;

void main() 
{
    vec4 diff=vec4(0);
    shade(diff);

    vec4 tex=texture(texSampler,inUV.xy);
    outColor=tex*diff;
}
