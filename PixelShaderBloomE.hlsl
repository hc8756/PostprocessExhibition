
cbuffer externalData : register(b0)
{
    float bloomThreshold;
    float bloomIntensity;
    float bloomSaturation;
    float bloomBlurSigma;
    float bloomBlurRadius;
    float bloomBlurStepSize;
    float pixelWidth;
    float pixelHeight;
}

// Defines the input to this pixel shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD;
};

// Textures and such
Texture2D pixels			: register(t0);
SamplerState samplerOptions	: register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{

    float4 color = pixels.Sample(samplerOptions, input.uv);
    float luminance = dot(color.rgb, float3(0.299, 0.587, 0.114)); // luminance is weighted sum of color
    float4 bloomColor = float4(0, 0, 0, 0);

    if (luminance > bloomThreshold)
    {
        bloomColor = color * (luminance - bloomThreshold) * bloomIntensity;
        bloomColor.rgb = bloomColor.rgb + (1.0f - bloomSaturation) * (color.rgb - bloomColor.rgb);
    }

    //  Gaussian blur
    const float offsets[15] = { -13.5f, -11.5f, -9.5f, -7.5f, -5.5f, -3.5f, -1.5f, 0, 1.5f, 3.5f, 5.5f, 7.5f, 9.5f, 11.5f, 13.5f };
    float4 blurColor = float4(0, 0, 0, 0);
    
    
    float blurSize = bloomBlurRadius * bloomBlurStepSize;
    float sigma = bloomBlurSigma * blurSize;
    for (int i = -bloomBlurRadius; i <= bloomBlurRadius; i++)
    {
        float weight = exp(-0.5f * i * i / (sigma * sigma));
        blurColor.rgb += pixels.Sample(samplerOptions, input.uv + float2(i, 0) * pixelWidth * offsets[i+bloomBlurRadius]).rgb * weight;
    }
    blurColor.rgb /= (2.0f * bloomBlurRadius + 1.0f);
    for (int i = -bloomBlurRadius; i <= bloomBlurRadius; i++)
    {
        float weight = exp(-0.5f * i * i / (sigma * sigma));
        blurColor.rgb += pixels.Sample(samplerOptions, input.uv + float2(0, i) * pixelHeight * offsets[i + bloomBlurRadius]).rgb * weight;
    }
    blurColor.rgb /= (2.0f * bloomBlurRadius + 1.0f);

    // Add the blurred bloom color to the input color
    color.rgb += blurColor.rgb;
    color.rgb = saturate(color.rgb);

    // Add the bloom color to the output color
    color.rgb += bloomColor.rgb;
    color.rgb = saturate(color.rgb);

    return color;
}