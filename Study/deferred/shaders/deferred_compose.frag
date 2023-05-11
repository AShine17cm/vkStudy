/* ��G-Buffer ������ɫ */
#version 450
#include "part.constants.shader"
layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

layout(set=0,binding=0) uniform sampler2DArray samplerShadowMap;//���������� samplerShadow
layout(set=0,binding=1) uniform sampler2D samplerPos;
layout(set=0,binding=2) uniform sampler2D samplerNormal;
layout(set=0,binding=3) uniform sampler2D samplerAlbedo;

struct Light{ mat4 mvp;vec4 vec;vec4 color;  };
layout(set=0,binding=4) uniform UniformObjectData
{
    mat4 proj;
    mat4 view;
    vec4 camera;
    ivec4 debugShadow;
    ivec4 debugDeferred;
    Light lights[LIGHT_COUNT];
}scene;

/* API�޷��ж�shader�е�if��������ʹʵ����û��ʹ�������λ�ϵ���Դ */
void main() 
{
    vec3 pos=texture(samplerPos,uv).rgb;
    vec3 normal=texture(samplerNormal,uv).rgb;
    vec4 albedo=texture(samplerAlbedo,uv);

    vec4 c=albedo;
    ivec4 mask=scene.debugDeferred;

    if(mask.x==1)
        outColor=vec4(pos,1.0);
    else if(mask.y==1)
        outColor=vec4(normal,1.0);
    else if(mask.z==1)
        outColor=albedo;
    else
        outColor=c;

}
