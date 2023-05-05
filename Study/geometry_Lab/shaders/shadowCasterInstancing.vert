//����UBO,ģ�;�����PushConstants
#version 450
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inUv;
layout(location = 4) in vec3 inColor;

layout(set=0,binding=0) uniform lightSpace
{
    mat4 vp;
}light;
/*------ ʵ���� ���� ------*/
struct InstanceData
{
    mat4 model;
    vec4 texIndex;
};//8*(16+4)
layout(set=1,binding=0) uniform uboInstance {
    InstanceData instances[1024*64/8/20];   //64*1024
}instances;

/*��� push_constant û��,ֻ��Ϊ�˱��� layout��һ��*/
layout(push_constant) uniform PushConstants
{
    mat4 model;
    vec4 tex;
} pushs;

void main() 
{
    mat4 model=instances.instances[gl_InstanceIndex].model;
    gl_Position=light.vp*model * vec4(inPos,1.0);
}
