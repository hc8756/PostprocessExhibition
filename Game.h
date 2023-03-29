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
#include "Exhibit.h"


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
	Material* CreateMaterial(const std::wstring* albedoPath, const std::wstring* normalsPath, const std::wstring* roughnessPath, const std::wstring* metalPath); // use null for defaults

	bool firstPerson;

	
	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//    Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Buffers to hold actual geometry data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	
	// Shaders and shader-related constructs
	SimplePixelShader* pixelShader;
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShaderSky;
	SimpleVertexShader* vertexShaderSky;
	SimplePixelShader* pixelShaderSobel;
	SimpleVertexShader* vertexShaderFull;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> defaultBlackSRV; // default for metal and roughness
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> defaultNormalSRV;

	int exhibitIndex;

	//Exhibit 1
	SimplePixelShader* pixelShaderBrightCont;//shader that effects brightness and contrast in post processing
	float brightness;
	float contrast;

	//Exhibit 2
	SimplePixelShader* pixelShaderBlur; //shader that effects blur in post processing
	int blur; // number of pixels to average outward from the current pixel

	//my models
	Mesh* cube;
	Mesh* sphere;


	//my game entities
	std::vector<GameEntity*> entityList = {};
	std::vector<Exhibit*> exhibits = {};

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

	// --------------------------------------------------------
	// Author: Chris Cascioli
	// Purpose: Creates a cube map on the GPU from 6 individual textures
	// 
	// - You are allowed to directly copy/paste this into your code base
	//   for assignments, given that you clearly cite that this is not
	//   code of your own design.
	//
	// - Note: This code assumes you’re putting the function in Game.cpp, 
	//   you’ve included WICTextureLoader.h and you have an ID3D11Device 
	//   ComPtr called “device”.  Make any adjustments necessary for
	//   your own implementation.
	// --------------------------------------------------------
	// Helper for creating a cubemap from 6 individual textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateCubemap(
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back);
};

