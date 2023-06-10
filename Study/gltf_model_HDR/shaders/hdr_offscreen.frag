#version 450

layout (set=0, binding = 0) uniform sampler2D samplerScr;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor0;
layout (location = 1) out vec4 outColor1;

layout (push_constant) uniform Pushs
{
	vec4 dataA;
	ivec4 dataB;
}pushs;

void main()
{
	vec4 color = texture(samplerScr, inUV);
	//���ݴ���� exposure �����ع�
	float exposure=pushs.dataA.x;
	//�߶�̬ ӳ�� �Ͷ�̬
	outColor0.rgb = vec3(1.0) - exp(-color.rgb * exposure);

	//��������
	float l = dot(outColor0.rgb, vec3(0.2126, 0.7152, 0.0722));
	float threshold =pushs.dataA.y;// 0.75;
	outColor1.rgb = (l > threshold) ? outColor0.rgb : vec3(0.0);
	outColor1.a = 1.0;
}