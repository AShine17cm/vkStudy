/* 用G-Buffer 进行着色 */
#version 450
#include "part.constants.shader"
#include "part.shadow_layers.frag"
layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

//layout(set=0,binding=0) uniform sampler2DArray samplerShadowMap;//此 sampler 在 part.shadows_layers.frag中
layout(set=0,binding=1) uniform sampler2D samplerPos;
layout(set=0,binding=2) uniform sampler2D samplerNormal;
layout(set=0,binding=3) uniform sampler2D samplerAlbedo;
/* layout 重复绑定也不会报错 */
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

/* 光照 */
vec3 shade(vec3 pos,vec3 normal)
{
    vec3 diff=vec3(0);
    for(int i=0;i<LIGHT_COUNT;i++)
    {
        Light lit=scene.lights[i];
        vec4 vecL=lit.vec;
        vec3  L=-vecL.xyz;              //平行光(指向光源)
        float dist=0;
        if(vecL.w>0.5)                  //点光源
        {
            L=vecL.xyz-pos;
            dist=length(L);
            L=normalize(L);
        }
       diff=diff+lit.color.rgb*lit.color.a* max(0,dot(L,normal));
    }
    return diff;
}
/* 采样阴影 */
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
/* API无法判定shader中的if条件，即使实际上没有使用这个等位上的资源 */
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
        outColor=albedo*vec4(diff,1)*shadow;//最终合成
    }
    else if(mask.x*mask.y*mask.z*mask.w==1)
    {
        outColor=vec4(diff,1)*shadow;       //阴影
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
