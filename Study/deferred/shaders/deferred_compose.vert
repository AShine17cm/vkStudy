#version 450

out gl_PerVertex
{
    vec4 gl_Position;
};

vec4 pts[6]=vec4[]
(
    vec4(-1,-1,0,0),    //вСио╫г
    vec4(1,-1,1,0),     //срио╫г
    vec4(-1,1,0,1),
       
    vec4(-1,1,0,1),     
    vec4(1,-1,1,0),
    vec4(1,1,1,1)
);

layout(location=0)out vec2 uv;

void main() 
{
    gl_Position=vec4(pts[gl_VertexIndex].xy,0.0,1.0);
    uv=pts[gl_VertexIndex].zw;
}
