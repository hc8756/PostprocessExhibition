#include "Sky.h"

Sky::Sky(Microsoft::WRL::ComPtr<ID3D11SamplerState> sso,
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> st,
	Mesh* sm,
	SimplePixelShader* sp,
	SimpleVertexShader* sv,
	Microsoft::WRL::ComPtr<ID3D11Device> device,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context
	)
{
	D3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.CullMode = D3D11_CULL_FRONT;
	device->CreateRasterizerState(&rastDesc, skyRasterizerState.GetAddressOf());

	D3D11_DEPTH_STENCIL_DESC depthDesc = {};
	depthDesc.DepthEnable = true;
	depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&depthDesc, skyDSS.GetAddressOf());
	//set resources
	skySamplerOptions = sso;
	skyTextureSRV = st;
	skyMesh = sm;
	skyPS = sp;
	skyVS = sv;
	skyDevice = device;
	skyContext = context;

}

void Sky::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, Camera* camera)
{
	// change necessary render states
	context->RSSetState(skyRasterizerState.Get());
	context->OMSetDepthStencilState(skyDSS.Get(), 0);

	// activate both shaders 
	skyVS->SetShader();
	skyPS->SetShader();

	//set pixel shader data
	skyPS->SetShaderResourceView("SkyTexture", skyTextureSRV);
	skyPS->SetSamplerState("BasicSamplerState", skySamplerOptions);

	//set vertex shader data
	skyVS->SetMatrix4x4("view", camera->GetView());
	skyVS->SetMatrix4x4("projection", camera->GetProjection());
	skyVS->CopyAllBufferData();

	//draw mesh
	skyMesh->Draw(context);

	// reset stuff
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);
}

Sky::~Sky()
{
	//sky is deleted in game class
}
