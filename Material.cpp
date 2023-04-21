#include "Material.h"

Material::Material(DirectX::XMFLOAT3 ct, float r, SimplePixelShader* ps, SimpleVertexShader* vs)
{
	colorTint = ct;
	roughness = r;
	pixelShader = ps;
	vertexShader = vs;
	transparency = 1.0f;
}

Material::~Material()
{
}

DirectX::XMFLOAT3 Material::GetColorTint()
{
	return colorTint;
}

float Material::GetRoughness()
{
	return roughness;
}

SimplePixelShader* Material::GetPixelShader()
{
	return pixelShader;
}

SimpleVertexShader* Material::GetVertexShader()
{
	return vertexShader;
}

void Material::SetMaterialColorTint(DirectX::XMFLOAT3 input)
{
	colorTint = input;
}

void Material::SetPixelShader(SimplePixelShader* ps)
{
	pixelShader = ps;
}

void Material::SetTransparency(float transparency)
{
	this->transparency = transparency;
}

float Material::GetTransparency()
{
	return transparency;
}

void Material::AddTextureSRV(std::string s, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
	textureSRVs.insert({s,srv});
}

void Material::AddSamplerState(std::string s, Microsoft::WRL::ComPtr<ID3D11SamplerState> ss)
{
	samplerStates.insert({ s,ss });
}

void Material::BindResources()
{
	for (auto& t : textureSRVs) { pixelShader -> SetShaderResourceView(t.first.c_str(), t.second); }
	for (auto& s : samplerStates) { pixelShader ->SetSamplerState(s.first.c_str(), s.second); }
}
