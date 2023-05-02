#version 450
#include "part.solid.frag"
layout(set=1,binding=1) uniform sampler2D texSampler;

void main() 
{
    float diff=0;
    float spec=0;
    shade(diff,spec);

    //float shadow=textureProj(shadowCoord,vec2(0,0));
    float shadow=filterPCF(shadowCoord);
    vec4 tex=texture(texSampler,inUV.xy);
    outColor=tex*(diff+spec)*shadow;
}
