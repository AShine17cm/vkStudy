#version 450
#include "part.solid.frag"

layout(set=1,binding=0) uniform samplerCube texSampler;

void main() 
{
    vec4 diff=vec4(0);
    shade(diff);

    vec3 normalWS=mat3x3(pushs.model)*inNormal;
    normalWS=normalize(normalWS);
    //relfect(in,normal)
    mat4 invModel=inverse(pushs.model);
    vec3 cI=normalize(inPos);
    vec3 cR=reflect(cI,normalWS);
    //cR=mat3x3(invModel)*cR;
    //cR.xy*=-1.0;

    vec4 tex=texture(texSampler,cR,0);
    outColor=tex*diff;
}
