#version 450

layout (set=0, binding = 0) uniform sampler2D samplerColor0;
layout (set=0, binding = 1) uniform sampler2D samplerColor1;

layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 outColor;

layout (push_constant) uniform Pushs
{
	vec4 dataA;
	ivec4 dataB;
}pushs;

void main(void)
{
	vec4	color = texture(samplerColor0, inUV);
	outColor = color;
}