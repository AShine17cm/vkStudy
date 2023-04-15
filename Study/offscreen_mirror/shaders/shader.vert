//场景UBO,模型矩阵用PushConstants
//使用gl_VertexIndex索引染色tint[4]
//使用descriptor数组中的一个(dataArray) 叠加染色
#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUv;
layout(location = 3) in vec3 inColor;

layout(set=0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;
layout(set=1,binding=0) uniform UboArray{
    vec4 data;
} dataArray[4];

layout(set=1,binding=1) uniform TintColor{
    vec4[4] tint;
} tint;

layout(push_constant) uniform PushConstants{
    mat4 model;
} pushs;

layout(location = 0) out vec3 fragColor;
layout(location=1)out vec2 uv;

void main() 
{
    gl_Position=mvp.proj * mvp.view * pushs.model * vec4(inPos,1.0);
    //fragColor = vec3(inColor.r,inColor.g,inColor.b);      //顶点色

    int idx=(gl_VertexIndex/4)%4;           //每4个点 取一个颜色
    fragColor=vec3(tint.tint[idx].rgb);   
    fragColor*=dataArray[0].data.rgb;
    uv=inUv;
}
