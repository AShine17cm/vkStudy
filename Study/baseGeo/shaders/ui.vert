#version 450
#extension GL_KHR_vulkan_glsl:enable

out gl_PerVertex
{
    vec4 gl_Position;
};

//(-1.0,-1.0)вСио╫г  (1.0,-1.0)срио╫г
layout(set=0,binding=0) uniform ui
{
    vec4 pts[6];
}ui_ubo;

layout(location=0)out vec2 uv;

void main() 
{
    gl_Position=vec4(ui_ubo.pts[gl_VertexIndex].xy,0.0,1.0);
    uv=ui_ubo.pts[gl_VertexIndex].zw;
}
