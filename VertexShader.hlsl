#include "ShaderIncludes.hlsli"
cbuffer ExternalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
	matrix worldInvTrans;

	matrix shadowView;
	matrix shadowProjection;
	matrix shadowProjection2;

}

VertexToPixel main( VertexShaderInput input )
{
	VertexToPixel output;
	matrix wvp = mul(projection, mul(view, world));
	output.position = mul(wvp, float4(input.position, 1.0f));
	
	output.uv = input.uv;

	output.normal = normalize(mul((float3x3)worldInvTrans, input.normal));
	output.tangent = normalize(mul((float3x3)worldInvTrans, input.tangent));
	output.worldPosition = mul(world, float4(input.position, 1.0f)).xyz;

	matrix shadowWVP = mul(shadowProjection, mul(shadowView, world));
	output.posForShadow = mul(shadowWVP, float4(input.position, 1.0f));

	return output;
}