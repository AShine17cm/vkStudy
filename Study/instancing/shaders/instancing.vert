//instancing使用额外的 vertex绑定,    模型矩阵,贴图数组-索引
//instancing data使用mat4 可能会超标
#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUv;
layout(location = 3) in vec3 inColor;

//instancing data  需要把矩阵mat4放到最后边，不然会overlapping use of location ...
layout(location = 4) in int instanceTexIndex;
layout(location = 5) in vec3 instanceColor;
layout(location = 6) in mat4 instanceModel;

layout(set=0, binding = 0) uniform uboScene {
    mat4 view;
    mat4 proj;
    vec3 lightPos;
} scene;

layout(location=0) out vec3 outNormal;
layout(location=1) out vec3 outLightVec;
layout(location=2) out vec3 outViewVec;
layout(location=3) out vec3 outUV;
layout(location=4) out vec3 outColor;
void main() 
{
    vec3 wldPos=(instanceModel*vec4(inPos,1.0)).xyz;
    gl_Position=scene.proj * scene.view * vec4(wldPos,1.0);

    outNormal=mat3(instanceModel)*inNormal;
    outLightVec=scene.lightPos-wldPos;
    outViewVec=-mat3(scene.view)*wldPos;
    outUV=vec3(inUv,instanceTexIndex);
    outColor=instanceColor;
}
