#include "GameEntity.h"

GameEntity::GameEntity(Mesh* mesh, Material* material)
{
	entityMesh = mesh;
	entityMaterial = material;

}

GameEntity::~GameEntity()
{
}

Mesh* GameEntity::GetMesh()
{
	return entityMesh;
}

Material* GameEntity::GetMaterial()
{
	return entityMaterial;
}

Transform* GameEntity::GetTransform()
{
	return &entityTransform;
}

void GameEntity::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,Camera* camera)
{
	entityMaterial->GetVertexShader()->SetShader(); 
	entityMaterial->GetPixelShader()->SetShader();

	SimpleVertexShader* vs = entityMaterial->GetVertexShader(); //   Simplifies next few lines 
	SimplePixelShader* ps = entityMaterial->GetPixelShader(); //   Simplifies next few lines 
	ps->SetFloat3("colorTint", entityMaterial->GetColorTint());
	ps->SetFloat("roughness", entityMaterial->GetRoughness());
	ps->SetFloat3("cameraPosition", camera->GetTransform()->GetPosition());
	vs->SetMatrix4x4("world", entityTransform.GetWorldMatrix());
	vs->SetMatrix4x4("view", camera->GetView());
	vs->SetMatrix4x4("projection", camera->GetProjection());
	vs->SetMatrix4x4("worldInvTrans", entityTransform.GetWorldInverseTranspose());
	vs->CopyAllBufferData();
	ps->CopyAllBufferData();
	//bind texture related resources
	entityMaterial->BindResources();
	entityMesh->Draw(context);
}

