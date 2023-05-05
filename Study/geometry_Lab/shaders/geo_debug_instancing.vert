#version 450
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inUv;
layout(location = 4) in vec3 inColor;
/* 传递实例的矩阵 */
layout(location=0)out vec4 model0;
layout(location=1)out vec4 model1;
layout(location=2)out vec4 model2;
layout(location=3)out vec4 model3;
layout(location=4)out vec3 normal;

/*------ 实例化 数据 ------*/
struct InstanceData
{
    mat4 model;
    vec4 texIndex;
};//8*(16+4)
layout(set=0,binding=0) uniform uboInstance 
{
    InstanceData instances[1024*64/8/20];   //64*1024
}instances;

void main() 
{
    mat4 M=instances.instances[gl_InstanceIndex].model;
    model0=M[0];
    model1=M[1];
    model2=M[2];
    model3=M[3];

    normal=inNormal;
    gl_Position=vec4(inPos,1.0);
}
