//#version 450
#include "part.constants.shader"
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inUv;
layout(location = 4) in vec3 inColor;

layout(location=0)out vec3 uv;
layout(location=1)out vec3 posWS;
layout(location=2)out vec3 normalWS;
layout(location=3)out vec3 tangentWS;

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

