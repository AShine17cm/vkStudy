#version 450
#include "part.solid.vert"

void main() 
{
    uv=vec3(inUv,pushs.tex.x);
    normal=mat3(pushs.model)*inNormal;
    posWS=(pushs.model*vec4(inPos,1.0)).xyz;
    gl_Position=scene.proj * scene.view * vec4(posWS,1.0);
    //shadowCoord=(biasMatrix*scene.lightSpace)*vec4(posWS,1.0);//阴影坐标(basis用于转到UV(0-1))
}
