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
        /* 采样阴影-多层 */
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
        if(scene.debugShadow[i]==0) layerVal=1.0;/* 3个光源的阴影,逐次展示，共同展示 */
        shadow[i]=layerVal;
    }

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
       diff=diff+lit.color.rgb*lit.color.a* max(0,dot(L,normal))*shadow[i];
    }
    return diff;
}
/* API无法判定shader中的if条件，即使实际上没有使用这个等位上的资源 */
void main() 
{
    vec3 pos=texture(samplerPos,uv).xyz;
    vec3 normal=texture(samplerNormal,uv).rgb;
    vec4 albedo=texture(samplerAlbedo,uv);

    vec3 diff=shade(pos,normal);
    ivec4 mask=scene.debugDeferred;

    if(mask.x+mask.y+mask.z+mask.w==0)
    {
        outColor=albedo*vec4(diff,1);//最终合成
    }
    else if(mask.x*mask.y*mask.z*mask.w==1)
    {
        outColor=vec4(diff,1);       //阴影
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
