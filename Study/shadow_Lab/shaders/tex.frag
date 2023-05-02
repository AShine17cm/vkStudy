#version 450
#include "part.solid.frag"
//#extension GL_KHR_vulkan_glsl:enable
layout(set=1,binding=0) uniform sampler2D texSampler;

void main() 
{
    float diff=0;
    float spec=0;
    shade(diff,spec);
    /* “ı”∞ */
    //float shadow=textureProj(shadowCoord,vec2(0,0));
    float shadow=filterPCF(shadowCoord);
    vec4 tex=texture(texSampler,inUV.xy);
    outColor=tex*(diff+spec)*shadow;
    //outColor=vec4(shadow);
    //outColor=shadowCoord;
}
