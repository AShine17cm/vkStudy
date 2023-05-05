//场景UBO,模型矩阵用PushConstants
#version 450
#include "part.solid.vert"

/*------ 实例化 数据 ------*/
struct InstanceData
{
    mat4 model;
    vec4 texIndex;
};//8*(16+4)
layout(set=1,binding=0) uniform uboInstance 
{
    InstanceData instances[1024*64/8/20];   //64*1024
}instances;

void main() 
{
    float texIndex=instances.instances[gl_InstanceIndex].texIndex.x;
    uv=vec3(inUv,texIndex);

    mat4 model=instances.instances[gl_InstanceIndex].model;
    normal=mat3(model)*inNormal;

    posWS=(model*vec4(inPos,1.0)).xyz;
    light=scene.light;
    camera=scene.camera;

    gl_Position=scene.proj * scene.view * vec4(posWS,1.0);
    shadowCoord=biasMatrix*scene.lightSpace*vec4(posWS,1.0);//阴影坐标(basis用于转到UV(0-1))
}
