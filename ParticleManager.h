#pragma once
class ParticleManager
{
public:
	ParticleManager(Microsoft::WRL::ComPtr<ID3D11Device> device);
	~ParticleManager();
	Transform emitterTransform;	
	Particle* particles;
	ParticleVertex* particleVertices;
	Microsoft::WRL::ComPtr<ID3D11Buffer> particleVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> particleIndexBuffer;
	// Data conversion info
	
	void CopyOneParticle(int index);
	void CopyParticlesToGPU();
	XMFLOAT3 CalcParticleVertexPos(int particleInd, int cornerInd);
	void UpdateSingleParticle(float dt, int index);
	void SpawnParticles();
	//Particle rendering 
private:
	int particleNum = 100;
	int livingParticleNum = 0;
	DirectX::XMFLOAT4 particleColor = XMFLOAT4(0.5f, 0.5f, 1.0f, 1.0f);
	DirectX::XMFLOAT3 particlesStartPos = XMFLOAT3(0.0f, 1.0f, 0.0f);
	float particleSize = 0.1;
	float velocityRange = 1;
	float lifeSpan = 10;
	int firstDeadParticle = 0;
	int firstLiveParticle = 0;
	float secondsPerParticle = 1.0f / 10;
	float timeSinceEmit = 0;
	DirectX::XMFLOAT2 DefaultUVs[4];
};

