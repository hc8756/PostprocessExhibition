#pragma once

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

class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

private:

	// Initialization helper methods - feel free to customize, combine, etc.
	void LoadShaders(); 
	void CreateBasicGeometry();
	void ResizePostProcessResources();



	
	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//    Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Buffers to hold actual geometry data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	
	// Shaders and shader-related constructs
	SimplePixelShader* pixelShader;
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShaderSky;
	SimpleVertexShader* vertexShaderSky;
	SimplePixelShader* pixelShaderSobel;
	SimpleVertexShader* vertexShaderFull;

	//my models
	Mesh* cube;
	Mesh* sphere;


	//my game entities
	std::vector<GameEntity*> entityList = {};

	//my materials
	std::vector<Material*> materialList = {};
	//Microsoft::WRL::ComPtr<ID3D11Buffer> constantBufferVS;

	Camera* camera;
	//int xDir = 1;
	//int yDir = 0;

	//create default normal and uv values
	DirectX::XMFLOAT3 defNormal;
	DirectX::XMFLOAT2 defUV;

	//lighting
	DirectX::XMFLOAT3 ambientColor;
	std::vector<Light> lightList = {};

	//sky 
	Sky* sky;

	// General post processing resources
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> ppRTV;		// Allows us to render to a texture
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ppSRV;		// Allows us to sample from the same texture

	// Outline rendering --------------------------
	Microsoft::WRL::ComPtr<ID3D11SamplerState> clampSampler;
	void PreRender();
	void PostRender();

	// Depth/normal technique
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> sceneDepthRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sceneDepthSRV;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> sceneNormalsRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sceneNormalsSRV;
};

