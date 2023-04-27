cbuffer ExternalData : register(b0)
{
	int blur;
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
	uint width;
	uint height;
	image.GetDimensions(width, height);

	float3 colorTotal = float3(0, 0, 0);
	int pixelCount = 0;
	for (int x = ceil(-blur / 2); x <= ceil(blur / 2); x++) {
		for (int y = ceil(-blur / 2); y <= ceil(blur / 2); y++) {
			int loadX = input.position.x + x;
			int loadY = input.position.y + y;
			if (loadX < 0 || loadY < 0 || loadX > width || loadY > height) {
				continue;
			}

			colorTotal += image.Load(int3(loadX, loadY, 0));
			pixelCount++;
		}
	}
	
	return float4(colorTotal, 1) / pixelCount;
}