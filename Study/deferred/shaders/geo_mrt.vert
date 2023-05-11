#version 450
#include "part.geo_mrt.vert"
/* �� ����,���ߣ�λ�� �������������ռ� */
void main() 
{
    mat3 toWS= transpose(inverse(mat3(pushs.model)));
    normalWS=toWS*inNormal;
    tangentWS=toWS*inTangent;
    posWS=(pushs.model*vec4(inPos,1.0)).xyz;
    uv=vec3(inUv,pushs.tex.x);

    gl_Position=scene.proj * scene.view * vec4(posWS,1.0);
}
