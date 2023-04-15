//https://www.saschawillems.de/blog/2016/08/13/vulkan-tutorial-on-rendering-a-fullscreen-quad-without-buffers/
#version 450
#extension GL_KHR_vulkan_glsl:enable

layout(location=0) out vec3 outUV;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main() 
{
    //0:<0,0> - 1:<1,0>
    //2:<0,1>
    outUV=vec3((gl_VertexIndex<<1)&2,gl_VertexIndex&2,gl_VertexIndex);
    //0   :<0,0>    -   clip:<1,0>
    //clip:<0,1>    -   clip:<1,1>
    //-------------------------
    //<0,0>     -       <inter,0>
    //<0,inter> -       <inter,inter>

    gl_Position=vec4(outUV.xy*2.0f-1.0f,0.0f,1.0f);
}
