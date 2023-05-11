#version 450

out gl_PerVertex
{
    vec4 gl_Position;
};

#define uiCount 2
//(-1.0,-1.0)左上角  (1.0,-1.0)右上角
layout(set=0,binding=0) uniform ui
{
    vec4 pts[6*uiCount];
    ivec4 debugShadow;
}ui_ubo;

layout(location=0)out vec2 uv;
//整形标量，或者整形向量时，变量必须使用flat修饰
layout(location=1)out flat int uiIdx;//那块 UI
void main() 
{
    gl_Position=vec4(ui_ubo.pts[gl_VertexIndex].xy,0.0,1.0);
    uv=ui_ubo.pts[gl_VertexIndex].zw;
    uiIdx=gl_VertexIndex/6;
}
