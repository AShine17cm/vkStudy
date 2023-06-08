#version 450
#include "part.constants.shader"
/* ������ת�����ƹ�ռ� inputAttribute��Ҫ��gltfģ��һ�� */
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV0;
layout (location = 3) in vec2 inUV1;
layout (location = 4) in vec4 inJoint0;
layout (location = 5) in vec4 inWeight0;
layout (location = 6) in vec4 inColor0;

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
    vec4 pos=pushs.model*vec4(inPos,1);
    pos.y=-pos.y; //gltf ��ģ�Ͷ���ͳһ��ת�� -Y����
    pos.xyz=pos.xyz/pos.w;
    gl_Position=vec4(pos.xyz,1.0);
}
