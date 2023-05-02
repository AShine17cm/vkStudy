#version 450
#include "part.solid.frag"

layout(set=1,binding=0) uniform sampler3D texSampler;

void main() 
{
    float diff=0;
    float spec=0;
    shade(diff,spec);

    float shadow=textureProj(shadowCoord,vec2(0,0));
    vec4 d=texture(texSampler,inUV);
    outColor=d*(diff+spec)*shadow;
}
