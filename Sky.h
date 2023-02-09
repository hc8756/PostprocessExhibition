#pragma once
#include "DXCore.h"
#include <wrl/client.h> 
#include "Mesh.h"
#include "SimpleShader.h"
#include "Camera.h"

class Sky {
public:
	Sky(Microsoft::WRL::ComPtr<ID3D11SamplerState> sso,
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> st,
		Mesh* sm,
		SimplePixelShader* sp,
		SimpleVertexShader* sv,
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);
	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,Camera* camera);
	~Sky();
private:
	//resources
	Microsoft::WRL::ComPtr<ID3D11SamplerState> skySamplerOptions;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skyTextureSRV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> skyDSS;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> skyRasterizerState;
	Mesh* skyMesh;
	SimplePixelShader* skyPS;
	SimpleVertexShader* skyVS;

	//for drawing
	Microsoft::WRL::ComPtr<ID3D11Device> skyDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> skyContext;
	
};