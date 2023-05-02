#include "ShaderIncludes.hlsli"

float4 main(ParticlePSInput input) : SV_TARGET
{
   return input.color;
}