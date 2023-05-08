//#version 450
/* ������ wrap + �߹� */
layout(location = 0) in vec3 inUV;
layout(location = 1) in vec3 inNormal;      //�󲿷�ʱ�� World-Space
layout(location = 2) in vec3 inPos;
layout(location = 3) in vec4 inLight;
layout(location = 4) in vec3 camera;
layout(location = 5) in vec4 shadowCoord;//��Ӱ��ͼ�е�����

layout(location = 0) out vec4 outColor;

layout(set= 0,binding= 1)   uniform sampler2D shadowMap;//�� scene��ص�ubo��ͬһ�� descriptorSet�У�������cmd�а�
//layout(set=1,binding=0)   uniform sampler2D texSampler;
//layout(set=1,binding=0)   uniform sampler3D texSampler;
//layout(set=1,binding=0)   uniform sampler2DArray samplerArray;
//layout(set=1,binding=0)   uniform samplerCube texSampler;

layout(push_constant) uniform PushConstants
{
    mat4 model;
    vec4 tex;
} pushs;

/*��Ӱ���*/
#define ambient 0.1
//������ rasterization����depthBiasEnable, ͬʱ���뵽dynamicState,Ȼ���� commandBuffer��̬����ƫ��
//#define zbias 0.000001  
float textureProj(vec4 shadowCoord,vec2 offset)//offset ����Soft-Shadow
{
    vec4 coord=shadowCoord/shadowCoord.w;       //�˲����Ǳ���ģ���Ȼcoord�������ֵ�����
    float shadow=1.0;
    float dist=texture(shadowMap,coord.xy+offset).r;
    if(coord.z>-1.0&&coord.z<1.0)
    {
        if(coord.w>0.0&&dist<coord.z)
        {
            shadow=ambient;
            //shadow=dist;
        }
    }
    return shadow;
}
float filterPCF(vec4 sc)
{
	ivec2 texDim = textureSize(shadowMap, 0);
	float scale = 1.5;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 1;
	
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			shadowFactor += textureProj(sc, vec2(dx*x, dy*y));
			count++;
		}
	}
	return shadowFactor / count;
}
/* �������� ����������� */
void shade(out float diff,out float spec) 
{
    vec3 vecLit = inLight.xyz;
    vec3 vecEye = camera - inPos;
    vec3 wldNormal =normalize( inNormal);
    vecEye = normalize(vecEye);
    /* ��������͸߹� */
    vec3 hVec = normalize(vecLit + vecEye);
    spec = max(0, dot(wldNormal, hVec));
    spec = pow(spec, 4);

    /* ������ wrap */
    diff = 0.02 + max(0, (dot(vecLit, wldNormal) + 0.4)) / 1.4 * inLight.w;
    //diff=max(0, dot(vecLit, wldNormal));
}
