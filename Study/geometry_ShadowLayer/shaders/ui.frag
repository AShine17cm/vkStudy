/* ����Ļ�ϻ�ͼ */
#version 450

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;
layout(set=0,binding=1) uniform sampler2D uiSampler;
layout(set=1,binding=0) uniform sampler2DArray arraySampler;

#define uiCount 2
layout(set=0,binding=0) uniform ui
{
    vec4 pts[6*uiCount];
    ivec4 debug;
}ui_ubo;

layout(push_constant) uniform pushConsts
{
    mat4 model;
    vec4 tex;
}pushs;

/* API�޷��ж�shader�е�if��������ʹʵ����û��ʹ�������λ�ϵ���Դ */
void main() 
{
    /* ����չʾ */
    if(pushs.tex.x<0.5)
    {
        outColor=texture(uiSampler,uv);
    }
    /* 3����Դ����Ӱ,���չʾ����ͬչʾ */
    else
    {
        outColor.r=texture(arraySampler,vec3(uv,0)).r*ui_ubo.debug.x;
        outColor.g=texture(arraySampler,vec3(uv,1)).r*ui_ubo.debug.y;
        outColor.b=texture(arraySampler,vec3(uv,2)).r*ui_ubo.debug.z;
    }
}
