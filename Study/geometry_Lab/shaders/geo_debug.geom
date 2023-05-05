#version 450
/* invocations ���ÿ������ִ�еĴ��� */
layout(triangles,invocations=1) in;
/* max_vt ����������� ���� 2=6 */
layout(line_strip,max_vertices=6) out;

layout(location=0) in vec3 inNormal[];
layout(location=0) out vec3 outColor;

/* �������� */
layout(set=0, binding = 0) uniform UniformBufferObject 
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
    float normalLength=0.2;
    /* ���������ÿһ������ ��Ϊ2��������һ���߶� */
    for(int i=0;i<gl_in.length();i++)
    {
        vec3 pos=gl_in[i].gl_Position.xyz;
        vec3 normal=inNormal[i];

        gl_Position=scene.proj*scene.view*pushs.model*vec4(pos,1.0);
        outColor=vec3(1.0,0,0);
        EmitVertex();

        gl_Position=scene.proj*scene.view*pushs.model*vec4(pos+normal*normalLength,1.0);
        outColor=vec3(0,0,1.0);
        EmitVertex();

        EndPrimitive();
    }

    
}
