#pragma once
#include "DXCore.h"
#include <DirectXMath.h>
#include <wrl/client.h>
#include "SimpleShader.h"

class Material
{
public:
	
	Material(DirectX::XMFLOAT3 ct, float r, SimplePixelShader* ps, SimpleVertexShader* vs);
	~Material();

	// Getters and Setters
	DirectX::XMFLOAT3 GetColorTint();
	float GetRoughness();
	SimplePixelShader* GetPixelShader();
	SimpleVertexShader* GetVertexShader();
	// Setters not necessary for shaders because they are loaded once
	void SetMaterialColorTint(DirectX::XMFLOAT3 input);
	void SetPixelShader(SimplePixelShader* ps);
	void SetTransparency(float transparency);
	float GetTransparency();

	//texture functions
	void AddTextureSRV(std::string s, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv);
	void AddSamplerState(std::string s, Microsoft::WRL::ComPtr<ID3D11SamplerState> ss);
	void BindResources();
private:
	DirectX::XMFLOAT3 colorTint;
	float roughness;
	float transparency;
	SimplePixelShader* pixelShader;
	SimpleVertexShader* vertexShader;
	
	//textuer related data
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplerStates;
};