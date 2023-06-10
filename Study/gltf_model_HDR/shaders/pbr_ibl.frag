#version 450
//#extension GL_KHR_vulkan_glsl:enable
#include "part.constants.shader"
#include "part.shadow_layers.frag"
#include "part.pbr.frag"

//World-Space
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;      
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec2 inUV1;
layout(location = 4) in vec4 inColor;

layout(location = 0) out vec4 outColor;


struct Light{ mat4 mvp;vec4 vec;vec4 color;};
layout(set=0,binding=0) uniform UniformObjectData
{
    mat4 proj;
    mat4 view;
    vec4 camera;
    ivec4 debug;
    Light lights[LIGHT_COUNT];
}scene;
//layout(set=0,binding=1)   uniform sampler2DArray shadowMap;

layout (set=1,binding = 0) uniform samplerCube samplerIrradiance;
layout (set=1,binding = 1) uniform sampler2D samplerBRDFLUT;
layout (set=1,binding = 2) uniform samplerCube prefilteredMap;

layout (set=2,binding=0) uniform PbrMaterial 
{
	vec4 baseColorFactor;	//金属流
	vec4 emissiveFactor;
	vec4 diffuseFactor;		//高光流
	vec4 specularFactor;

	int isMetallic;
	int occlusionTextureSet;
	int emissiveTextureSet;

	float metallicFactor;	
	float roughnessFactor;	
	float alphaMask;	
	float alphaMaskCutoff;

    //高级参数
    float exposure;
	float gamma;
	float prefilteredCubeMipLevels;
	float scaleIBLAmbient;
	float debugViewInputs;
	float debugViewEquation;
} mat;

layout (set = 2, binding = 1) uniform sampler2D colorMap;
layout (set = 2, binding = 2) uniform sampler2D physicalDescriptorMap;
layout (set = 2, binding = 3) uniform sampler2D normalMap;
layout (set = 2, binding = 4) uniform sampler2D aoMap;
layout (set = 2, binding = 5) uniform sampler2D emissiveMap;

#define ALBEDO vec3(pbrBasic.rgba.rgb)
#define MANUAL_SRGB 1
const float c_MinRoughness = 0.04;


// http://filmicgames.com/archives/75
vec3 Uncharted2Tonemap(vec3 color)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	float W = 11.2;
	return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
}

vec4 tonemap(vec4 color)
{
	vec3 outcol = Uncharted2Tonemap(color.rgb * mat.exposure);
	outcol = outcol * (1.0f / Uncharted2Tonemap(vec3(11.2f)));	
	return vec4(pow(outcol, vec3(1.0f / mat.gamma)), color.a);
}
//色彩 转线性空间
vec4 SRGBtoLINEAR(vec4 srgbIn)
{
	#ifdef MANUAL_SRGB
	#ifdef SRGB_FAST_APPROXIMATION
	vec3 linOut = pow(srgbIn.xyz,vec3(2.2));
	#else //SRGB_FAST_APPROXIMATION
	vec3 bLess = step(vec3(0.04045),srgbIn.xyz);
	vec3 linOut = mix( srgbIn.xyz/vec3(12.92), pow((srgbIn.xyz+vec3(0.055))/vec3(1.055),vec3(2.4)), bLess );
	#endif //SRGB_FAST_APPROXIMATION
	return vec4(linOut,srgbIn.w);;
	#else //MANUAL_SRGB
	return srgbIn;
	#endif //MANUAL_SRGB
}

//切线方向 是 UV 的梯度差，UV 变化最大的方向
vec3 getNormal()
{
	//法线扰动, http://www.thetenthplanet.de/archives/1180
	vec3 tangentNormal = texture(normalMap, inUV).xyz * 2.0 - 1.0;

	vec3 q1 = dFdx(inPos);
	vec3 q2 = dFdy(inPos);
	vec2 st1 = dFdx(inUV);
	vec2 st2 = dFdy(inUV);

	vec3 N = normalize(inNormal);
	vec3 T = normalize(q1 * st2.t - q2 * st1.t);
	vec3 B = -normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);

	return normalize(TBN * tangentNormal);
}
//IBL 的 漫反射/高光
vec3 getIBLContribution(PBRInfo pbrInputs, vec3 n, vec3 reflection)
{
	//IBL 的漫反射
	vec3 diffuseLight = SRGBtoLINEAR(tonemap(texture(samplerIrradiance, n))).rgb;
	vec3 diffuse = diffuseLight * pbrInputs.diffuseColor;
	diffuse *= mat.scaleIBLAmbient;

	//IBL 的高光
	float lod = (pbrInputs.perceptualRoughness * mat.prefilteredCubeMipLevels);//粗糙度 映射 辐照图
	vec3 specularLight = SRGBtoLINEAR(tonemap(textureLod(prefilteredMap, reflection, lod))).rgb;//高光第一部分 辐照度

	vec3 brdf = (texture(samplerBRDFLUT, vec2(pbrInputs.NdotV, 1.0 - pbrInputs.perceptualRoughness))).rgb;//Lut 表中的坐标
	vec3 specular = specularLight * (pbrInputs.specularColor * brdf.x + brdf.y);//高光第二部分 预计算的 F0
	specular *= mat.scaleIBLAmbient;

	return diffuse + specular;
}

vec3 diffuseFunc(PBRInfo pbrInputs)
{
	return pbrInputs.diffuseColor / PI;
}
//F 函数 菲尼尔反射
vec3 specularReflection(PBRInfo pbrInputs)
{
	return pbrInputs.reflectance0 + (pbrInputs.reflectance90 - pbrInputs.reflectance0) * pow(clamp(1.0 - pbrInputs.VdotH, 0.0, 1.0), 5.0);
}
//G 函数 几何遮挡
float geometricOcclusion(PBRInfo pbrInputs)
{
	float NdotL = pbrInputs.NdotL;
	float NdotV = pbrInputs.NdotV;
	float r = pbrInputs.alphaRoughness;

	float attenuationL = 2.0 * NdotL / (NdotL + sqrt(r * r + (1.0 - r * r) * (NdotL * NdotL)));
	float attenuationV = 2.0 * NdotV / (NdotV + sqrt(r * r + (1.0 - r * r) * (NdotV * NdotV)));
	return attenuationL * attenuationV;
}
//D 法线分布
float microfacetDistribution(PBRInfo pbrInputs)
{
	float roughnessSq = pbrInputs.alphaRoughness * pbrInputs.alphaRoughness;
	float f = (pbrInputs.NdotH * roughnessSq - pbrInputs.NdotH) * pbrInputs.NdotH + 1.0;
	return roughnessSq / (PI * f * f);
}
//将 高光参数 转换为 金属度参数
float convertMetallic(vec3 diffuse, vec3 specular, float maxSpecular) 
{
	float perceivedDiffuse = sqrt(0.299 * diffuse.r * diffuse.r + 0.587 * diffuse.g * diffuse.g + 0.114 * diffuse.b * diffuse.b);
	float perceivedSpecular = sqrt(0.299 * specular.r * specular.r + 0.587 * specular.g * specular.g + 0.114 * specular.b * specular.b);
	if (perceivedSpecular < c_MinRoughness) {
		return 0.0;
	}
	float a = c_MinRoughness;
	float b = perceivedDiffuse * (1.0 - maxSpecular) / (1.0 - c_MinRoughness) + perceivedSpecular - 2.0 * c_MinRoughness;
	float c = c_MinRoughness - perceivedSpecular;
	float D = max(b * b - 4.0 * a * c, 0.0);
	return clamp((-b + sqrt(D)) / (2.0 * a), 0.0, 1.0);
}

/* 世界坐标 计算光照向量 */
vec4 shade( ) 
{
	float perceptualRoughness;
	float metallic;
	vec3 diffuseColor;
	vec4 baseColor;

	vec3 f0 = vec3(0.04);

	//baseColor = SRGBtoLINEAR(texture(colorMap,inUV )) * mat.baseColorFactor;

	//metallic/////// 金属流
	if(mat.isMetallic==1)
	{
		perceptualRoughness = mat.roughnessFactor;
		metallic = mat.metallicFactor;

		// 粗糙度 g  金属度 b   OC (可选) r
		vec4 mrSample = texture(physicalDescriptorMap, inUV);
		perceptualRoughness = mrSample.g * perceptualRoughness;
		metallic = mrSample.b * metallic;

		// The albedo may be defined from a base texture or a flat color
		baseColor = SRGBtoLINEAR(texture(colorMap, inUV)) * mat.baseColorFactor;
	}

	//specular///////  高光流
	if(mat.isMetallic==0)
	{
		//光泽度 转 粗糙度
		vec4 pPixel=texture(physicalDescriptorMap, inUV);
		perceptualRoughness = 1.0 - pPixel.a;
		const float epsilon = 1e-6;

		vec4 diffuse = SRGBtoLINEAR(texture(colorMap, inUV));
		vec3 specular = SRGBtoLINEAR(pPixel).rgb;
		float maxSpecular = max(max(specular.r, specular.g), specular.b);

		//光泽度 转为 金属度
		metallic = convertMetallic(diffuse.rgb, specular, maxSpecular);

		vec3 baseColorDiffusePart = diffuse.rgb * ((1.0 - maxSpecular) / (1 - c_MinRoughness) / max(1 - metallic, epsilon)) * mat.diffuseFactor.rgb;
		vec3 baseColorSpecularPart = specular - (vec3(c_MinRoughness) * (1 - metallic) * (1 / max(metallic, epsilon))) * mat.specularFactor.rgb;
		baseColor = vec4(mix(baseColorDiffusePart, baseColorSpecularPart, metallic * metallic), diffuse.a);
	}
	baseColor *= inColor;
	diffuseColor = baseColor.rgb * (vec3(1.0) - f0);
	diffuseColor *= 1.0 - metallic;
	float alphaRoughness = perceptualRoughness * perceptualRoughness;
	vec3 specularColor = mix(f0, baseColor.rgb, metallic);//F0
	//反射比
	float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);
	float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
	vec3 specularEnvironmentR0 = specularColor.rgb;
	vec3 specularEnvironmentR90 = vec3(1.0, 1.0, 1.0) * reflectance90;

	vec3 n = getNormal();
	vec3 v = normalize(scene.camera.xyz - inPos);    // Vector from surface point to camera
	vec3 L=scene.lights[0].vec.xyz;					//平行光(指向光源)
    L.y=-L.y;										//灯光 翻转到 -Y轴, 和顶点的操作一致
	vec3 l = normalize(L);							// Vector from surface point to light
	vec3 h = normalize(l+v);                        // Half vector between both l and v
	vec3 reflection = -normalize(reflect(v, n));
	reflection.y *= -1.0f;

	float NdotL = clamp(dot(n, l), 0.001, 1.0);
	float NdotV = clamp(abs(dot(n, v)), 0.001, 1.0);
	float NdotH = clamp(dot(n, h), 0.0, 1.0);
	float LdotH = clamp(dot(l, h), 0.0, 1.0);
	float VdotH = clamp(dot(v, h), 0.0, 1.0);

	PBRInfo pbrInputs = PBRInfo(NdotL,NdotV,NdotH,LdotH,VdotH,
		perceptualRoughness,
		metallic,
		specularEnvironmentR0,
		specularEnvironmentR90,
		alphaRoughness,
		diffuseColor,
		specularColor
	);

	//反射方程的 F G D
	vec3 F = specularReflection(pbrInputs);
	float G = geometricOcclusion(pbrInputs);
	float D = microfacetDistribution(pbrInputs);

	const vec3 u_LightColor = scene.lights[0].color.rgb*scene.lights[0].color.a;
	//直接光  漫反射
	vec3 diffuseContrib = (1.0 - F) * diffuseFunc(pbrInputs);
	//直接光  高光
	vec3 specContrib = F * G * D / (4.0 * NdotL * NdotV);
	//直接光
	vec3 color = NdotL * u_LightColor * (diffuseContrib + specContrib);
	//IBL 的漫反射+高光
	color += getIBLContribution(pbrInputs, n, reflection);

	//OC 遮蔽
	if (mat.occlusionTextureSet > -1) 
	{
		float ao = texture(aoMap, (mat.occlusionTextureSet == 0 ? inUV : inUV1)).r;
		color =  color * ao;
	}
	//outColor = vec4(color, baseColor.a);

    /* 采样阴影 */
    vec4 coord=scene.lights[0].mvp* vec4(inPos,1.0);
    float shadow= filterPCF(coord,0);
    if(scene.debug[0]==0) shadow=1.0;
	color*=shadow;
	//自发光
	if (mat.emissiveTextureSet > -1)
	{
		vec3 emissive = SRGBtoLINEAR(texture(emissiveMap, inUV)).rgb;
		color += emissive;
	}

    vec4 finalColor= vec4(color,baseColor.a);

	//调试 纹理
	if (mat.debugViewInputs > 0.0) 
	{
		int index = int(mat.debugViewInputs);
		float roughX=0;
		float metalX=0;

		if(mat.isMetallic==0)//高光流的 调试信息 转 金属度
		{
			roughX = 1.0 - texture(physicalDescriptorMap, inUV).a;// a通道 光泽度
			vec4 diffX = SRGBtoLINEAR(texture(colorMap, inUV));
			vec3 specX = SRGBtoLINEAR(texture(physicalDescriptorMap, inUV)).rgb;
			float maxSpecX = max(max(specX.r, specX.g), specX.b);
			//光泽度 转为 金属度
			metalX = convertMetallic(diffX.rgb, specX, maxSpecX);
		}else
		{
			vec4 samX = texture(physicalDescriptorMap, inUV);
			roughX=samX.g;
			metalX=samX.b;
		}
		switch (index) {
			case 1:
				finalColor.rgba = texture(colorMap, inUV );
				break;
			case 2:
				finalColor.rgb = texture(normalMap,inUV).rgb; // normalize(inNormal);
				break;
			case 3:
				finalColor.rgb = (mat.occlusionTextureSet > -1) ? texture(aoMap, inUV).rrr : vec3(0.0f);
				break;
			case 4:
				finalColor.rgb = (mat.emissiveTextureSet > -1) ? texture(emissiveMap, inUV).rgb : vec3(0.0f);
				break;
			case 5:
				finalColor.rgb =vec3(metalX);
				//finalColor.rgb = texture(physicalDescriptorMap, inUV).bbb;//金属度
				break;
			case 6:
				finalColor.rgb =vec3(roughX);
				//finalColor.rgb =texture(physicalDescriptorMap, inUV).ggg;//粗糙度
				break;
		}
		finalColor = SRGBtoLINEAR(finalColor);
	}
	//调试 BRDF 方程
	// "none", "Diff (l,n)", "F (l,h)", "G (l,v,h)", "D (h)", "Specular"
	if (mat.debugViewEquation > 0.0) 
	{
		int index = int(mat.debugViewEquation);
		switch (index) {
			case 1:
				finalColor.rgb = diffuseContrib;
				break;
			case 2:
				finalColor.rgb = F;
				break;
			case 3:
				finalColor.rgb = vec3(G);
				break;
			case 4: 
				finalColor.rgb = vec3(D);
				break;
			case 5:
				finalColor.rgb = specContrib;
				break;				
		}
	}

	return finalColor;
}
void main() 
{
    outColor=shade();
}
