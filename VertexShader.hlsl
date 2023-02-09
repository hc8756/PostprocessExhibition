#include "ShaderIncludes.hlsli"
cbuffer ExternalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
	matrix worldInvTrans;

}

VertexToPixel main( VertexShaderInput input )
{
	VertexToPixel output;
	matrix wvp = mul(projection, mul(view, world));
	output.position = mul(wvp, float4(input.position, 1.0f));
	output.worldPosition = mul(world, float4(input.position, 1.0f)).xyz;
	
	output.uv = input.uv;

	output.normal = normalize(mul((float3x3)worldInvTrans, input.normal));
	output.tangent = normalize(mul((float3x3)worldInvTrans, input.tangent));
	output.worldPosition = mul(world, float4(input.position, 1.0f)).xyz;

	return output;
}