/* 在屏幕上画图 */
#version 450

layout(location = 0) in vec2 uv;
layout(location = 1) in flat int uiIdx;//整形in/out需要使用<flat>修饰
layout(location = 0) out vec4 outColor;
layout(set=0,binding=1) uniform sampler2D uiSampler;
layout(set=0,binding=2) uniform sampler2DArray arraySampler;

#define uiCount 2
layout(set=0,binding=0) uniform ui
{
    vec4 pts[6*uiCount];
    ivec4 debugShadow;
}ui_ubo;

void main() 
{
    /* 正常展示 */
    if(uiIdx==0)
    {
        outColor=texture(uiSampler,uv);
    }
    /* 3个光源的阴影,逐次展示，共同展示 */
    if(uiIdx==1)
    {
        outColor.r=texture(arraySampler,vec3(uv,0)).r*ui_ubo.debugShadow.x;
        outColor.g=texture(arraySampler,vec3(uv,1)).r*ui_ubo.debugShadow.y;
        outColor.b=texture(arraySampler,vec3(uv,2)).r*ui_ubo.debugShadow.z;
    }
}
