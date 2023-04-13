
cbuffer externalData : register(b0)
{
    float bloomThreshold;
    float bloomIntensity;
    float bloomSaturation;
    float bloomBlurSigma;
    float bloomBlurRadius;
    float bloomBlurStepSize;
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

    // Sample the input texture
    float4 color = pixels.Sample(samplerOptions, input.uv);

    // Compute the average luminance of the input texture
    float luminance = dot(color.rgb, float3(0.299, 0.587, 0.114));

    // Compute the bloom color
    float4 bloomColor = float4(0, 0, 0, 0);
    if (luminance > bloomThreshold)
    {
        bloomColor = color * (luminance - bloomThreshold) * bloomIntensity;
        bloomColor.rgb = bloomColor.rgb + (1.0f - bloomSaturation) * (color.rgb - bloomColor.rgb);
    }

    // Blur the bloom color using a separable Gaussian blur
    float4 blurColor = float4(0, 0, 0, 0);
    float blurSize = bloomBlurRadius * bloomBlurStepSize;
    float sigma = bloomBlurSigma * blurSize;
    for (int i = -bloomBlurRadius; i <= bloomBlurRadius; i++)
    {
        float weight = exp(-0.5f * i * i / (sigma * sigma));
        blurColor.rgb += pixels.Sample(samplerOptions, input.uv + float2(i, 0) * bloomBlurStepSize).rgb * weight;
    }
    blurColor.rgb /= (2.0f * bloomBlurRadius + 1.0f);
    for (int i = -bloomBlurRadius; i <= bloomBlurRadius; i++)
    {
        float weight = exp(-0.5f * i * i / (sigma * sigma));
        blurColor.rgb += pixels.Sample(samplerOptions, input.uv + float2(0, i) * bloomBlurStepSize).rgb * weight;
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