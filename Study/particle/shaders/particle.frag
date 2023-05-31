#version 450

layout (location = 0) out vec4 outFragColor;

void main() 
{
    float rotCenter = 0.5;
	vec2 rotUV = vec2(
		 (gl_PointCoord.x - rotCenter) +	(gl_PointCoord.y - rotCenter) + rotCenter,
		 (gl_PointCoord.y - rotCenter) -	(gl_PointCoord.x - rotCenter) + rotCenter);

	outFragColor=vec4(rotUV,1,1);
}
