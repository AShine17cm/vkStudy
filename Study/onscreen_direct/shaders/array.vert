//不使用 vertex buffer,直接编码到 vertex.shader中
#version 450
#extension GL_KHR_vulkan_glsl:enable

layout(location=0) out vec3 fragColor;

out gl_PerVertex
{
    vec4 gl_Position;
};
//Pipeline 设定是 顺时针
vec2 positions[9]=vec2[]
(
    vec2(-1.0,-1.0),    //左上角
    vec2(0,0),
    vec2(-1.0,0),

    vec2(1.0,-1.0),     //右上角
    vec2(1.0,0),
    vec2(0,0),

    vec2(1.0,0.5),
    vec2(1.0,1.0),
    vec2(0.5,1.0)
);
vec3 colors[9]=vec3[]
(
    vec3(1.0,0.0,0.0),
    vec3(0.0,1.0,0.0),
    vec3(0.0,0.0,1.0),

    vec3(1.0,0.0,0.0),
    vec3(0.0,1.0,0.0),
    vec3(0.0,0.0,1.0),

    vec3(1.0,0.0,0.0),
    vec3(0.0,1.0,0.0),
    vec3(0.0,0.0,1.0)
);

void main() 
{
    gl_Position=vec4(positions[gl_VertexIndex],0.0,1.0);
    fragColor=colors[gl_VertexIndex];
    //fragColor=vec3(gl_VertexIndex*0.333333);
}
