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
        /* ������Ӱ-��� */
    float shadow[LIGHT_COUNT];
    for(int i=0;i<LIGHT_COUNT;i+=1)
    {
        vec4 coord=scene.lights[i].mvp* vec4(pos,1.0);
        float layerVal=1.0;
        #ifdef SHADOW_PCF
            layerVal= filterPCF(coord,i);
        #else
            layerVal= textureProj(coord,i,vec2(0));
        #endif
        if(scene.debugShadow[i]==0) layerVal=1.0;/* 3����Դ����Ӱ,���չʾ����ͬչʾ */
        shadow[i]=layerVal;
    }

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
       diff=diff+lit.color.rgb*lit.color.a* max(0,dot(L,normal))*shadow[i];
    }
    return diff;
}
/* API�޷��ж�shader�е�if��������ʹʵ����û��ʹ�������λ�ϵ���Դ */
void main() 
{
    vec3 pos=texture(samplerPos,uv).xyz;
    vec3 normal=texture(samplerNormal,uv).rgb;
    vec4 albedo=texture(samplerAlbedo,uv);

    vec3 diff=shade(pos,normal);
    ivec4 mask=scene.debugDeferred;

    if(mask.x+mask.y+mask.z+mask.w==0)
    {
        outColor=albedo*vec4(diff,1);//���պϳ�
    }
    else if(mask.x*mask.y*mask.z*mask.w==1)
    {
        outColor=vec4(diff,1);       //��Ӱ
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
