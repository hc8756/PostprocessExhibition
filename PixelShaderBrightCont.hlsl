cbuffer ExternalData : register(b0)
{
	float brightness;
	float contrast;
}

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

// Textures and such
Texture2D image			: register(t0);
SamplerState samplerOptions	: register(s0);


// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	//https://stackoverflow.com/questions/944713/help-with-pixel-shader-effect-for-brightness-and-contrast
	float3 finalColor=((image.Sample(samplerOptions, input.uv).rgb - 0.5f)* max(contrast, 0)) + 0.5f;
	finalColor +=float3(brightness, brightness, brightness);
	return float4(finalColor, 1);
}