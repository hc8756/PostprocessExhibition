#include "ParticleManager.h"
#include "DXCore.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include "Mesh.h"
#include "BufferStructs.h"
#include "GameEntity.h"
#include <vector>
#include "Camera.h"
#include "Material.h"
#include "Lights.h"
#include "Sky.h"
#include "Exhibit.h"
#include <stdlib.h>
#include <optional>
#include "SpriteBatch.h"
#include "ParticleManager.h"
#include "Particle.h"


ParticleManager::ParticleManager(Microsoft::WRL::ComPtr<ID3D11Device> device)
{
	//create c++ data structure containing particles
	particles = new Particle[particleNum];
	emitterTransform.SetPosition(particlesStartPos.x, particlesStartPos.y, particlesStartPos.z);

	for (int i = 0; i < particleNum; i++) {
		XMFLOAT3 newStartVelocity = XMFLOAT3(0, 0, 0);
		newStartVelocity.x += (((float)rand() / RAND_MAX) * 2 - 1) * 1;
		newStartVelocity.y += (((float)rand() / RAND_MAX) * 2 - 1) * 1;
		newStartVelocity.z += (((float)rand() / RAND_MAX) * 2 - 1) * 1;
		particles[i] = { particlesStartPos, newStartVelocity, 0 };
	}

	//Use this to fill vertex and index buffers for particles
	DefaultUVs[0] = XMFLOAT2(0, 0);
	DefaultUVs[1] = XMFLOAT2(1, 0);
	DefaultUVs[2] = XMFLOAT2(1, 1);
	DefaultUVs[3] = XMFLOAT2(0, 1);

	// Create UV's here, as those will usually stay the same
	particleVertices = new ParticleVertex[4 * particleNum];
	for (int i = 0; i < particleNum * 4; i += 4)
	{
		particleVertices[i + 0].UV = DefaultUVs[0];
		particleVertices[i + 1].UV = DefaultUVs[1];
		particleVertices[i + 2].UV = DefaultUVs[2];
		particleVertices[i + 3].UV = DefaultUVs[3];
	}

	// DYNAMIC vertex buffer (no initial data necessary)
	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbDesc.Usage = D3D11_USAGE_DYNAMIC;
	vbDesc.ByteWidth = sizeof(ParticleVertex) * 4 * particleNum;
	device->CreateBuffer(&vbDesc, 0, particleVertexBuffer.GetAddressOf());

	// Index buffer data
	unsigned int* indices = new unsigned int[particleNum * 6];
	int indexCount = 0;
	for (int i = 0; i < particleNum * 4; i += 4)
	{
		indices[indexCount++] = i;
		indices[indexCount++] = i + 1;
		indices[indexCount++] = i + 2;
		indices[indexCount++] = i;
		indices[indexCount++] = i + 2;
		indices[indexCount++] = i + 3;
	}
	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = indices;

	// Regular (static) index buffer
	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.ByteWidth = sizeof(unsigned int) * particleNum * 6;
	device->CreateBuffer(&ibDesc, &indexData, particleIndexBuffer.GetAddressOf());
	delete[] indices;
}

ParticleManager::~ParticleManager()
{
	delete[] particles;
	delete[] particleVertices;
}

void ParticleManager::UpdateSingleParticle(float dt, int index)
{
	// Check for valid particle age before doing anything
	if (particles[index].Age >= lifeSpan)
		return;

	// Update and check for death
	particles[index].Age += dt;
	if (particles[index].Age >= lifeSpan)
	{
		// Recent death, so retire by moving alive count
		firstLiveParticle++;
		firstLiveParticle %= particleNum;
		livingParticleNum--;
		return;
	}

	// Adjust the position
	XMVECTOR startPos = XMLoadFloat3(&particles[index].Position);
	XMVECTOR startVel = XMLoadFloat3(&particles[index].StartVelocity);

	float t = particles[index].Age;

	// Use constant acceleration function
	XMStoreFloat3(&particles[index].Position, startVel * t + startPos);
}

void ParticleManager::CopyParticlesToGPU()
{
	if (firstLiveParticle < firstDeadParticle)
	{
		for (int i = firstLiveParticle; i < firstDeadParticle; i++)
			CopyOneParticle(i);
	}
	else
	{
		// Update first half (from firstAlive to max particles)
		for (int i = firstLiveParticle; i < particleNum; i++)
			CopyOneParticle(i);

		// Update second half (from 0 to first dead)
		for (int i = 0; i < firstDeadParticle; i++)
			CopyOneParticle(i);
	}

	D3D11_MAPPED_SUBRESOURCE mapped = {};
	context->Map(particleVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	memcpy(mapped.pData, particleVertices, sizeof(ParticleVertex) * 4 * particleNum);
	context->Unmap(particleVertexBuffer.Get(), 0);
}

void ParticleManager::SpawnParticles()
{
	if (livingParticleNum == particleNum)
		return;

	// Reset the first dead particle
	particles[firstDeadParticle].Age = 0;
	//particles[firstDeadParticle].Size = startSize;
	//particles[firstDeadParticle].Color = startColor;

	particles[firstDeadParticle].Position = particlesStartPos;
	// Increment and wrap
	firstDeadParticle++;
	firstDeadParticle %= particleNum;

	livingParticleNum++;
}

void ParticleManager::UpdateParticles(float dt)
{
	if (firstLiveParticle < firstDeadParticle)
	{
		for (int i = firstLiveParticle; i < firstDeadParticle; i++) {
			UpdateSingleParticle(dt, i);
		}
	}
	else {
		for (int i = firstLiveParticle; i < particleNum; i++) {
			UpdateSingleParticle(dt, i);
		}

		// Update second half (from 0 to first dead)
		for (int i = 0; i < firstDeadParticle; i++) {
			UpdateSingleParticle(dt, i);
		}
	}
	timeSinceEmit += dt;

	// Enough time to emit?
	while (timeSinceEmit > secondsPerParticle)
	{
		SpawnParticles();
		timeSinceEmit -= secondsPerParticle;
	}
}

void ParticleManager::CopyOneParticle(int index)
{
	int i = index * 4;

	particleVertices[i + 0].Position = CalcParticleVertexPos(index, 0);
	particleVertices[i + 1].Position = CalcParticleVertexPos(index, 1);
	particleVertices[i + 2].Position = CalcParticleVertexPos(index, 2);
	particleVertices[i + 3].Position = CalcParticleVertexPos(index, 3);

	particleVertices[i + 0].Color = particleColor;
	particleVertices[i + 1].Color = particleColor;
	particleVertices[i + 2].Color = particleColor;
	particleVertices[i + 3].Color = particleColor;
}


XMFLOAT3 ParticleManager::CalcParticleVertexPos(int particleIndex, int quadCornerIndex)
{
	// Get the right and up vectors out of the view matrix
	XMFLOAT4X4 view = camera->GetView();
	XMVECTOR camRight = XMVectorSet(view._11, view._21, view._31, 0);
	XMVECTOR camUp = XMVectorSet(view._12, view._22, view._32, 0);

	// Determine the offset of this corner of the quad
	// Since the UV's are already set when the emitter is created, 
	// we can alter that data to determine the general offset of this corner
	XMFLOAT2 offset = DefaultUVs[quadCornerIndex];
	offset.x = offset.x * 2 - 1;	// Convert from [0,1] to [-1,1]
	offset.y = (offset.y * -2 + 1);	// Same, but flip the Y

	// Load into a vector, which we'll assume is float3 with a Z of 0
	// Create a Z rotation matrix and apply it to the offset
	XMVECTOR offsetVec = XMLoadFloat2(&offset);
	XMMATRIX rotMatrix = XMMatrixRotationZ(0);
	offsetVec = XMVector3Transform(offsetVec, rotMatrix);

	// Add and scale the camera up/right vectors to the position as necessary
	XMVECTOR posVec = XMLoadFloat3(&particles[particleIndex].Position);
	posVec += camRight * XMVectorGetX(offsetVec) * particleSize;
	posVec += camUp * XMVectorGetY(offsetVec) * particleSize;

	// This position is all set
	XMFLOAT3 pos;
	XMStoreFloat3(&pos, posVec);
	return pos;
}

void ParticleManager::DrawParticles() {
	// Particle states
	context->OMSetBlendState(particleBlendState.Get(), 0, 0xffffffff);	// Additive blending
	context->OMSetDepthStencilState(particleDepthState.Get(), 0);		// No depth WRITING

	CopyParticlesToGPU();

	// Draw buffer
	UINT stride = sizeof(ParticleVertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, particleVertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(particleIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	vertexShaderParticle->SetShader();
	pixelShaderParticle->SetShader();
	vertexShaderParticle->SetMatrix4x4("world", particleTransform.GetWorldMatrix());
	vertexShaderParticle->SetMatrix4x4("view", camera->GetView());
	vertexShaderParticle->SetMatrix4x4("projection", camera->GetProjection());
	vertexShaderParticle->CopyAllBufferData();

	if (firstLiveParticle < firstDeadParticle)
	{
		context->DrawIndexed(livingParticleNum * 6, firstLiveParticle * 6, 0);
	}
	else
	{
		// Draw first half (0 -> dead)
		context->DrawIndexed(firstDeadParticle * 6, 0, 0);

		// Draw second half (alive -> max)
		context->DrawIndexed((particleNum - firstLiveParticle) * 6, firstLiveParticle * 6, 0);
	}

	// Reset states
	context->OMSetBlendState(0, 0, 0xffffffff);
	context->OMSetDepthStencilState(0, 0);
	context->RSSetState(0);
}