#version 450
#include "part.solid.frag"

layout(set=1,binding=0) uniform sampler2DArray samplerArray;

void main() 
{
    vec4 diff=vec4(0);
    shade(diff);

    vec4 tex=texture(samplerArray,inUV);
    outColor=tex*diff;
}
