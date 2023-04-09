#include "ShaderIncludes.hlsli"
/*
Texture2D SurfaceTexture  : register(t0); // "t" registers for textures
Texture2D SpecularMap  : register(t1);
Texture2D NormalMap  : register(t2);*/
Texture2D Albedo:  register(t0); 
Texture2D NormalMap:  register(t1); 
Texture2D RoughnessMap   :  register(t2); 
Texture2D MetalnessMap   :  register(t3);
SamplerState BasicSamplerState : register(s0); // "s" registers for samplers

cbuffer ExternalData : register(b0)
{
	float3 colorTint;
	float roughness;
	float3 ambientColor;
	int numCels;
	float3 cameraPosition;
	Light lights[5];
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

	for (int i = 0; i < 5; i++) {
		//directional light
		if (lights[i].Type == 0) {
			totalLight+= DirectionalLight(lights[i], input.normal, input.worldPosition, cameraPosition, roughness, metalness, surfaceColor, specularColor);
		}
		else if (lights[i].Type == 1) {
			totalLight+= PointLight(lights[i], input.normal, input.worldPosition, cameraPosition, roughness, metalness, surfaceColor, specularColor);
		}
	}

	// for cel-shading, round total light to certain values
	if(numCels > 0) {
		totalLight = floor(totalLight * numCels) / numCels;
	}

	return float4(pow(totalLight+ambientTerm,1.0f/2.2f), 1);

}


