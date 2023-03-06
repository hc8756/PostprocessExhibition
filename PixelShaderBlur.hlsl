cbuffer ExternalData : register(b0)
{
	int blur;
	float pixelWidth;
	float pixelHeight;
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
	float3 colorTotal = float3(0, 0, 0);
	for (int x = -blur; x <= blur; x++) {
		for (int y = -blur; y <= blur; y++) {
			colorTotal += image.Load(int3(input.position.x + x, input.position.y + y, 0));
		}
	}
	
	int pixelCount = (2 * blur + 1) * (2 * blur + 1);
	return float4(colorTotal, 1) / pixelCount;
}