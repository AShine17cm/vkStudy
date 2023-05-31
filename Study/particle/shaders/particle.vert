#version 450
layout (location = 0) in vec4 inPos;
layout (location = 1) in vec4 inColor;
layout (location = 2) in float inSize;

layout (location = 0) out vec4 outColor;

layout (binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 modelview;
	vec2 viewportDim;
	float pointSize;
} ubo;

out gl_PerVertex
{
	vec4 gl_Position;
	float gl_PointSize;
};

void main() 
{
	outColor = inColor;
	gl_Position = ubo.projection * ubo.modelview * vec4(inPos.xyz, 1.0);	
	
	// Base size of the point sprites
	float spriteSize = 8.0 * inSize;

	// Scale particle size depending on camera projection
	vec4 eyePos = ubo.modelview * vec4(inPos.xyz, 1.0);
	vec4 projectedCorner = ubo.projection * vec4(0.5 * spriteSize, 0.5 * spriteSize, eyePos.z, eyePos.w);
	gl_PointSize = ubo.viewportDim.x * projectedCorner.x / projectedCorner.w;	
}
