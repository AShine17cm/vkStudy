/* ��G-Buffer ������ɫ */
#version 450
#include "part.constants.shader"
#include "part.shadow_layers.frag"
layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

//layout(set=0,binding=0) uniform sampler2DArray samplerShadowMap;//�� sampler �� part.shadows_layers.frag��
layout(set=0,binding=1) uniform sampler2D samplerPos;
layout(set=0,binding=2) uniform sampler2D samplerNormal;
layout(set=0,binding=3) uniform sampler2D samplerAlbedo;
/* layout �ظ���Ҳ���ᱨ�� */
//layout(set=0,binding=3) uniform sampler2D samplerAlbedoX;
//layout(set=1,binding=4) uniform sampler2DArray samplerAlbedoY;

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

/* ���� */
vec3 shade(vec3 pos,vec3 normal)
{
    vec3 diff=vec3(0);
    for(int i=0;i<LIGHT_COUNT;i++)
    {
        Light lit=scene.lights[i];
        vec4 vecL=lit.vec;
        vec3  L=-vecL.xyz;              //ƽ�й�(ָ���Դ)
        float dist=0;
        if(vecL.w>0.5)                  //���Դ
        {
            L=vecL.xyz-pos;
            dist=length(L);
            L=normalize(L);
        }
       diff=diff+lit.color.rgb*lit.color.a* max(0,dot(L,normal));
    }
    return diff;
}
/* ������Ӱ */
float texShadow(vec3 pos)
{
    vec4 shadowCoord=scene.lights[0].mvp*vec4(pos,1.0);
    float shadow=1.0;
    for(int i=0;i<LIGHT_COUNT;i++)
    {
        vec4 shadowCoord=scene.lights[i].mvp*vec4(pos,1.0);
        float shadow_i=filterPCF(shadowCoord,i);
        if(scene.debugShadow[i]==1)
            shadow=shadow*shadow_i;
    }
    return shadow;
}
/* API�޷��ж�shader�е�if��������ʹʵ����û��ʹ�������λ�ϵ���Դ */
void main() 
{
    vec3 pos=texture(samplerPos,uv).xyz;
    vec3 normal=texture(samplerNormal,uv).rgb;
    vec4 albedo=texture(samplerAlbedo,uv);

    vec3 diff=shade(pos,normal);
    float shadow=texShadow(pos);

    ivec4 mask=scene.debugDeferred;

    if(mask.x+mask.y+mask.z+mask.w==0)
    {
        outColor=albedo*vec4(diff,1)*shadow;//���պϳ�
    }
    else if(mask.x*mask.y*mask.z*mask.w==1)
    {
        outColor=vec4(diff,1)*shadow;       //��Ӱ
    }
    else if(mask.x==1)
        outColor=vec4(pos,1.0);
    else if(mask.y==1)
        outColor=vec4(normal,1.0);
    else if(mask.z==1)
        outColor=albedo;
    else if(mask.w==1)
        outColor=vec4(diff,1);

}
