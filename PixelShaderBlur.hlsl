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
	int blurTest = 3;
	for (int x = -blurTest; x <= blurTest; x++) {
		for (int y = -blurTest; y <= blurTest; y++) {
			colorTotal += image.Sample(samplerOptions, input.uv + float2(x * pixelWidth, y * pixelHeight)).rgb;
			//colorTotal += image.Load(int3(input.position.x + x, input.position.y + y, 0));
		}
	}
	int pixelCount = (2 * blurTest + 1) * (2 * blurTest + 1);

	/*int sideLength = 2 * blur + 1;
	int pixelCount = sideLength * sideLength;
	for (int i = 0; i < pixelCount; i++) {
		int x = -blur + i % sideLength;
		int y = -blur + i / sideLength;
		colorTotal += image.Sample(samplerOptions, input.uv + float2(x * pixelWidth, y * pixelHeight)).rgb;
	}*/
	
	return float4(colorTotal, 1) / pixelCount;
	//return float4(blur, blur, blur, 1);
}