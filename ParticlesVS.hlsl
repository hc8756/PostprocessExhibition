#include "ShaderIncludes.hlsli"
cbuffer ExternalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
}


ParticlePSInput main(ParticleVSInput input)
{
	ParticlePSInput output;
	matrix wvp = mul(projection, mul(view, world));
	output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));
	output.uv = input.uv;
	output.color = input.color;
	return output;
}