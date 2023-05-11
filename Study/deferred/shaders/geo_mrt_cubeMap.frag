#version 450
#include "part.geo_mrt.frag"

layout(set=1,binding=1) uniform samplerCube texSampler;

void main() 
{
    vec3 T=normalize(inTangent);
    vec3 N=normalize(inNormal);
    vec3 B=cross(N,T);
    mat3 TBN=mat3(T,B,N);
    vec3 normal_tbn=texture(samplerNormal,inUV.xy).xyz*2.0-vec3(1.0);
    normal_tbn=TBN*normalize(normal_tbn);

    outNormal=vec4(normal_tbn,1.0);
    outPos=vec4(inPos,1.0);

    vec3 cI=normalize(inPos);
    vec3 cR=reflect(cI,normal_tbn);
    outAlbedo=texture(texSampler,cR,0);
}
