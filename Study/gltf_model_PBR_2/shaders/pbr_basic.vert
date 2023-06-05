#version 450
#include "part.constants.shader"
/* 将顶点转换到灯光空间 inputAttribute需要和gltf模型一致 */
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV0;
layout (location = 3) in vec2 inUV1;
layout (location = 4) in vec4 inJoint0;
layout (location = 5) in vec4 inWeight0;
layout (location = 6) in vec4 inColor0;

layout(location=0)out vec3 uv;
layout(location=1)out vec3 normal;
layout(location=2)out vec3 posWS;
//layout(location=3)out vec3 outColor;

struct Light{ mat4 mvp;vec4 vec;vec4 color;  };
layout(set=0,binding=0) uniform UniformObjectData
{
    mat4 proj;
    mat4 view;
    vec4 camera;
    ivec4 debug;
    Light lights[LIGHT_COUNT];
}scene;

layout(push_constant) uniform PushConstants
{
    mat4 model;
    vec4 tex;
} pushs;

void main() 
{
    uv=vec3(inUV0,pushs.tex.w);
    normal=normalize(transpose(inverse(mat3(pushs.model)))*inNormal);
    vec4 pos=pushs.model*vec4(inPos,1.0);
    pos.y=-pos.y;
    posWS=pos.xyz/pos.w;
    gl_Position=scene.proj * scene.view * vec4(posWS,1.0);
}
