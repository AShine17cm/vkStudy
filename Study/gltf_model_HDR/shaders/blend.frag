#version 450

layout (set=0, binding = 0) uniform sampler2D samplerScr;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

layout (push_constant) uniform Pushs
{
	vec4 dataA;
	ivec4 dataB;
}pushs;

void main()
{
	outColor = texture(samplerScr, inUV);
}