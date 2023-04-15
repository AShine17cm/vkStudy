//场景UBO,模型矩阵用PushConstants
//使用gl_VertexIndex索引染色tint[4]

#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUv;
layout(location = 3) in vec3 inColor;

layout(set=0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 lightPos;
} scene;

layout(set=1,binding=0) uniform uboMat{
    vec4 tint;
} tint;

layout(push_constant) uniform PushConstants{
    mat4 model;
    float visible;
} pushs;

layout(location = 0) out vec3 outColor;

void main() 
{
    gl_Position=scene.proj * scene.view * pushs.model * vec4(inPos,1.0);
    outColor=vec3(tint.tint.rgb);  
}
