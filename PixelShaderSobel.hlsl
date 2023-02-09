cbuffer ExternalData : register(b0)
{
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
	float horizontalWeights[9] =
	{
		-1, +0, +1,
		-1, +0, +1,
		-1, +0, +1
	};

	float verticalWeights[9] =
	{
		-1, -1, -1,
		+0, +0, +0,
		+1, +1, +1
	};

	// Take 9 pixels from the texture, itself + neighbors
	float3 pixels[9];
	for (int i = 0; i < 9; i++) {
		pixels[i]= image.Sample(samplerOptions, input.uv + float2(horizontalWeights[i]* pixelWidth, verticalWeights[i] *pixelHeight)).rgb;
	}
	float3 horizTotal = float3(0, 0, 0);
	float3 vertTotal = float3(0, 0, 0);

	for (int i = 0; i < 9; i++)
	{
		horizTotal += pixels[i] * horizontalWeights[i];
		vertTotal += pixels[i] * verticalWeights[i];
	}

	float3 mag= sqrt(horizTotal * horizTotal + vertTotal * vertTotal);
	float sobel = saturate(mag.x + mag.y + mag.z);

	float3 finalColor = lerp(image.Sample(samplerOptions, input.uv).rgb, float3(0, 0, 0), sobel);

	return float4(finalColor, 1);
}