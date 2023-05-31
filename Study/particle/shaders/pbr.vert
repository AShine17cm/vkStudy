#version 450
#include "part.pbr.vert"

void main() 
{
    uv=vec3(inUV0,pushs.tex.x);
    normal=normalize(transpose(inverse(mat3(pushs.model)))*inNormal);
    vec4 pos=pushs.model*vec4(inPos,1.0);
    pos.y=-pos.y;
    posWS=pos.xyz/pos.w;
    gl_Position=scene.proj * scene.view * vec4(posWS,1.0);
}
