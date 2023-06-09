#version 450

layout (location = 0) out vec2 outUV;

out gl_PerVertex
{
	vec4 gl_Position;
};
//Pipeline 设定是 顺时针
vec4 positions[6]=vec4[]
(
    vec4(-1.0,-1.0,0,0),    //左上角
    vec4(-1.0,1.0,0,1),
    vec4(1.0,1.0,1,1),
       
    vec4(-1.0,-1.0,0,0),     //右上角
    vec4(1.0,1.0,1,1),
    vec4(1.0,-1.0,1,0)
);

void main() 
{
    gl_Position=vec4(positions[gl_VertexIndex].xy,0.0,1.0);
    outUV=positions[gl_VertexIndex].zw;
}
