#include "ShaderIncludes.hlsli"

Texture2D Albedo		 :  register(t0); 
Texture2D NormalMap		 :  register(t1); 
Texture2D RoughnessMap   :  register(t2); 
Texture2D MetalnessMap   :  register(t3);
Texture2D ShadowMap		 :  register(t4);
Texture2D ShadowMap2	 :  register(t5);
SamplerState BasicSamplerState : register(s0); // "s" registers for samplers
SamplerComparisonState ShadowSampler	: register(s1); // special sampler for shadows

cbuffer ExternalData : register(b0)
{
	// Scene related
	Light lights[5];

	float3 ambientColor;

	// Camera related
	int numCels;
	float3 cameraPosition;

	// Material related
	float3 colorTint;
	float2 uvScale;
	float2 uvOffset;
}

float4 main(VertexToPixel input) : SV_TARGET
{
	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);

	//unpack normal
	float3 unpackedNormal = NormalMap.Sample(BasicSamplerState, input.uv).rgb * 2 - 1;
	//form 3x3 rotation matrix
	float3 N = normalize(input.normal);  
	float3 T = normalize(input.tangent); 
	T = normalize(T - N * dot(T, N)); 
	float3 B = cross(T, N); 
	float3x3 TBN = float3x3(T, B, N);
	//transform normal
	input.normal = normalize(mul(unpackedNormal, TBN));

	//start light calculations
	float3 totalLight = 0.0f;
	
	//surface color
	float3 surfaceColor = pow( Albedo.Sample(BasicSamplerState, input.uv).rgb,2.2f);
	surfaceColor *= colorTint;
	float3 ambientTerm = ambientColor * surfaceColor;

	//roughness map
	float roughness = RoughnessMap.Sample(BasicSamplerState, input.uv).r;

	//metalness map
	float metalness = MetalnessMap.Sample(BasicSamplerState, input.uv).r;

	//specular
	float3 specularColor = lerp(F0_NON_METAL.rrr, surfaceColor.rgb, metalness);
	
	// SHADOW MAPPING --------------------------------
	// Note: This is only for a SINGLE light!  If you want multiple lights to cast shadows,
	// you need to do all of this multiple times IN THIS SHADER.
	float shadowAmount;
	float shadowAmount2;

	float2 shadowUV = input.posForShadow.xy / input.posForShadow.w * 0.5f + 0.5f;
	shadowUV.y = 1.0f - shadowUV.y;

	float2 shadowUV2 = input.posForShadow2.xy / input.posForShadow2.w * 0.5f + 0.5f;
	shadowUV2.y = 1.0f - shadowUV2.y;

	// Calculate this pixel's depth from the light
	float depthFromLight = input.posForShadow.z / input.posForShadow.w;
	float depthFromLight2 = input.posForShadow2.z / input.posForShadow2.w;

	shadowAmount = ShadowMap.SampleCmpLevelZero(ShadowSampler, shadowUV, depthFromLight);
	shadowAmount2 = ShadowMap2.SampleCmpLevelZero(ShadowSampler, shadowUV2, depthFromLight2);
	shadowAmount = min(shadowAmount, shadowAmount2);

	totalLight += DirectionalLight(lights[0], input.normal, input.worldPosition, cameraPosition, roughness, metalness, surfaceColor, specularColor);
	totalLight *= (lights[0].CastsShadows ? shadowAmount: 1.0f);


	// for cel-shading, round total light to certain values
	if(numCels > 0) {
		totalLight = round(totalLight * numCels) / numCels;
	}

	return float4(pow(totalLight+ambientTerm,1.0f/2.2f), 1);
}


