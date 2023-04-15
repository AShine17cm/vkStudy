#version 450

layout(set=1,binding=1) uniform sampler2D texSampler;

layout(location=0) in vec3 inNormal;
layout(location=1) in vec3 inLightVec;
layout(location=2) in vec3 inViewVec;
layout(location=3) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() 
{
    vec4 tex=texture(texSampler,uv);

    vec3 N=normalize(inNormal);
    vec3 L=normalize(inLightVec);
    vec3 V=normalize(inViewVec);
    vec3 R=reflect(-L,N);
    vec3 diffuse=max(dot(N,L),0.25)*tex.rgb;
    vec3 specular=pow(max(dot(R,V),0.0),8.0)*vec3(0.75);
    outColor=vec4(diffuse+specular,1.0);

}
