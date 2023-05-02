#version 450
#include "part.solid.vert"

void main() 
{
    normal=mat3(pushs.model)*inNormal;
    posWS=(pushs.model*vec4(inPos,1.0)).xyz;
    light=scene.light;
    camera=scene.camera;

    uv=normalize(inPos)*0.5+vec3(1,1,1)*0.5;
    uv.xy=normalize(uv.xy);
    uv.z=(posWS.y+3.2)/6;//cube 队列的长度

    gl_Position=scene.proj * scene.view * vec4(posWS,1.0);
    shadowCoord=(biasMatrix*scene.lightSpace)*vec4(posWS,1.0);
}
