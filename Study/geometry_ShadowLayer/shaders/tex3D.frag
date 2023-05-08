#version 450
#include "part.solid.frag"

layout(set=1,binding=0) uniform sampler3D texSampler;

void main() 
{
    vec4 diff=vec4(0);
    shade(diff);

    vec4 d=texture(texSampler,inUV);
    outColor=d*diff;
}
