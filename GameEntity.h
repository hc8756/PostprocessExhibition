#pragma once
#include "DXCore.h"
#include <DirectXMath.h>
#include <wrl/client.h> 
#include "Transform.h"
#include "Mesh.h"
#include "BufferStructs.h"
#include "Camera.h"
#include "material.h"
using namespace DirectX;
class GameEntity {
public:
	GameEntity(Mesh* mesh, Material* material);
	~GameEntity();
	Transform* GetTransform();
	Mesh* GetMesh();
	Material* GetMaterial();
	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, Camera* camera);

private:
	Transform entityTransform;
	Mesh* entityMesh;
	Material* entityMaterial;
};