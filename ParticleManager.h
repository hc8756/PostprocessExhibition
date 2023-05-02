#pragma once
#include "DXCore.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include "BufferStructs.h"
#include "GameEntity.h"
#include <vector>
#include "Camera.h"
#include <stdlib.h>
#include <optional>
#include "Particle.h"
#include "Material.h"

class ParticleManager
{
public:
	ParticleManager(Microsoft::WRL::ComPtr<ID3D11Device> device, XMFLOAT3 startPos);
	~ParticleManager();
	Microsoft::WRL::ComPtr<ID3D11Buffer> particleVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> particleIndexBuffer;
	void UpdateParticles(float dt);
	void DrawParticlesInternal(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, Camera* camera, SimplePixelShader* ps, SimpleVertexShader* vs);
	void CopyParticlesToGPU(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, Camera* camera);
	int livingParticleNum = 0;
	float lifeSpan = 3;
	int firstDeadParticle = 0;
	int firstLiveParticle = 0;
	float particleSize = 0.1;
	float velocityRange = 1;
	float particlesPerSecond = 10.0f;
	float secondsPerParticle = 1.0f / particlesPerSecond;
	float timeSinceEmit = 0;
	DirectX::XMFLOAT4 particleColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	int particleNum = 1000;

private:
	DirectX::XMFLOAT2 DefaultUVs[4];
	XMFLOAT3 particlesStartPos = XMFLOAT3(0.0f, 1.0f, 0.0f);
	Transform emitterTransform;	
	Particle* particles;
	ParticleVertex* particleVertices;
	XMFLOAT3 CalcParticleVertexPos(int particleInd, int cornerInd, Camera* camera);
	void CopyOneParticle(int index, Camera* camera);
	void UpdateSingleParticle(float dt, int index);
	void SpawnParticles();
	
	
};

