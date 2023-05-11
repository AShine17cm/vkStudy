#version 450
#include "part.constants.shader"
layout(triangles,invocations=LIGHT_COUNT) in;
layout(triangle_strip,max_vertices=3) out;

struct Light{ mat4 mvp;vec4 vec;vec4 color;  };
layout(set=0,binding=0) uniform UniformObjectData
{
    mat4 proj;
    mat4 view;
    vec4 camera;
    ivec4 debugShadow;
    ivec4 debugDeferred;
    Light lights[LIGHT_COUNT];
}scene;

layout(push_constant) uniform PushConstants
{
    mat4 model;
    vec4 tex;
} pushs;

//main Ö´ÐÐ´ÎÊý LIGHT_COUNT
void main() 
{
    for(int i=0;i<gl_in.length();i++)
    {
        gl_Layer=gl_InvocationID;
        vec4 posWS=gl_in[i].gl_Position;  //World Space
        gl_Position=scene.lights[gl_InvocationID].mvp*posWS;
        EmitVertex();
    }
    EndPrimitive();
}
