#version 450

out gl_PerVertex
{
	vec4 gl_Position;
	float gl_PointSize;
};

layout(push_constant) uniform constants
{
    mat4 mvp;
    vec4 pos;
    vec4 color;
}consts;

layout(location=0)out vec4 color;

void main() 
{
    vec3 pos=consts.pos.xyz;
    pos.y=-pos.y;

    gl_Position=consts.mvp* vec4(pos,1.0);
    gl_PointSize=consts.color.a;
    color=consts.color;
}
