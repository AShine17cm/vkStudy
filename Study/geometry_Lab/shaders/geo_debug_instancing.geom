#version 450
/* invocations ���ÿ������ִ�еĴ��� */
layout(triangles,invocations=1) in;
/* max_vt ����������� ���� 2=6 */
layout(line_strip,max_vertices=6) out;
/* ����ֱ�Ӵ��� ���� */
layout(location=0) in vec4 model0[];
layout(location=1) in vec4 model1[];
layout(location=2) in vec4 model2[];
layout(location=3) in vec4 model3[];
layout(location=4) in vec3 inNormal[];


layout(location=0) out vec3 outColor;

/* �������� */
layout(set=0, binding = 1) uniform UniformBufferObject 
{
    mat4 view;
    mat4 proj;
    mat4 lightSpace;// �� world-Space ����ת���� light-Space
    vec4 light;
    vec3 camera;
} scene;

layout(push_constant) uniform pushConsts
{
    mat4 model;
    vec4 tex;
}pushs;

void main() 
{
    /* ʵ���ľ��� */
    mat4 model={model0[0],model1[0],model2[0],model3[0]};

    float normalLength=0.2;
    /* ���������ÿһ������ ��Ϊ2��������һ���߶� */
    for(int i=0;i<gl_in.length();i++)
    {
        vec3 pos=gl_in[i].gl_Position.xyz;
        vec3 normal=inNormal[i];

        gl_Position=scene.proj*scene.view*model*vec4(pos,1.0);
        outColor=vec3(1.0,0,0);
        EmitVertex();

        gl_Position=scene.proj*scene.view*model*vec4(pos+normal*normalLength,1.0);
        outColor=vec3(0,0,1.0);
        EmitVertex();

        EndPrimitive();
    }

    
}