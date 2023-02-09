#include "ShaderIncludes.hlsli"
cbuffer ExternalData : register(b0)
{
	matrix view;
	matrix projection;
}

VertexToPixelSky main(VertexShaderInput input)
{
	VertexToPixelSky output;

	matrix viewNoTranslation = view;
	viewNoTranslation._14 = 0; 
	viewNoTranslation._24 = 0; 
	viewNoTranslation._34 = 0;

	output.position = mul(projection, mul(viewNoTranslation, float4(input.position,1.0f)));
	output.position.z = output.position.w;
	output.sampleDir = input.position;
	return output;
}