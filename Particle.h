#pragma once

#include <DirectXMath.h>

struct Particle
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 StartVelocity;
	float Age;
};

struct ParticleVertex
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT2 UV;
	DirectX::XMFLOAT4 Color;
};