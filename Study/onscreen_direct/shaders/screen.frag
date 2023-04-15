#version 450

layout(location = 0) in vec3 inUV;

layout(location = 0) out vec4 outColor;

void main() 
{
    outColor=vec4(inUV.x,inUV.y,0.0,1.0);
    //float k=inUV.z;
    //outColor=vec4(k*k*k*k);
}
