#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inUv;
layout(location = 4) in vec3 inColor;

//layout (location = 0) in vec3 inPos;
//layout (location = 1) in vec3 inNormal;
//layout (location = 2) in vec2 inUV;

//layout (binding = 0) uniform UBO 
//{
//	mat4 projection;
//	mat4 model;
//	vec4 viewPos;
//	float lodBias;
//} ubo;

/* ≥°æ∞æÿ’Û */
layout(set=0, binding = 0) uniform UniformBufferObject 
{
    mat4 view;
    mat4 proj;
    vec4 light;
    vec3 camera;
} scene;

layout(push_constant) uniform PushConstants
{
    mat4 model;
    vec4 tex;
} pushs;

layout (location = 0) out vec2 outUV;
layout (location = 1) out float outLodBias;

void main() 
{
	outUV = inUv;
	outLodBias =0;// ubo.lodBias;
	//gl_Position = ubo.projection * ubo.model * vec4(inPos.xyz, 1.0);
    vec3 wldPos=(pushs.model*vec4(inPos,1.0)).xyz;
    gl_Position=scene.proj * scene.view * vec4(wldPos,1.0);
}
