#include "ShaderIncludes.hlsli"
TextureCube SkyTexture  : register(t0); 
SamplerState BasicSamplerState : register(s0);

cbuffer ExternalData : register(b0)
{
	float3 colorTint;
	float roughness;
	float3 ambientColor;
	float3 cameraPosition;
	Light lights[5];
}

float4 main(VertexToPixelSky input) : SV_TARGET
{
	
	return SkyTexture.Sample(BasicSamplerState, input.sampleDir);

}


