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
#include <stdlib.h>
#include <optional>
#include "SpriteBatch.h"


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
	void CreateShadowMapResources();
	void RenderShadowMap();
	Material* CreateMaterial(const wchar_t* albedoPath, const wchar_t* normalsPath, const wchar_t* roughnessPath, const wchar_t* metalPath); // use nullptr for defaults
	Material* CreateColorMaterial(XMFLOAT3 color);

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
	SimpleVertexShader* vertexShaderShadow;
	SimplePixelShader* pixelShaderSky;
	SimpleVertexShader* vertexShaderSky;
	SimplePixelShader* pixelShaderSobel;
	SimpleVertexShader* vertexShaderFull;
	SimplePixelShader* pixelShaderBloomE;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> defaultBlackSRV; // default for metal and roughness
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> defaultNormalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pureWhiteSRV;

	int exhibitIndex;

	//Exhibit 0
	SimplePixelShader* pixelShaderBrightCont;//shader that effects brightness and contrast in post processing
	float brightness;
	float contrast;

	//Exhibit 1
	SimplePixelShader* pixelShaderBlur; //shader that effects blur in post processing
	int blur; // number of pixels to average outward from the current pixel

	// Exhibit 2
	bool useSobel;

	// Exhibit 3
	bool useBloom=0;
	float bloomThreshold=1.0;
	float bloomIntensity=0.5;
	float bloomSaturation=1.0;
	float bloomBlurSigma=2.0;
	float bloomBlurRadius=5.00;
	float bloomBlurStepSize=0.09;
	
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

	
	DirectX::XMFLOAT3 ambientColor;
	std::vector<Light> lightList = {};
	int numCels; // for cel shading, 0 denotes regular shading
	//sky 
	Sky* sky;

	//Sprite batch resources
	std::shared_ptr<DirectX::SpriteBatch> spriteBatch;

	// General post processing resources
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> ppRTV;		// Allows us to render to a texture
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ppSRV;		// Allows us to sample from the same texture
	Microsoft::WRL::ComPtr<ID3D11SamplerState> ppSampler;
	// Outline rendering --------------------------
	Microsoft::WRL::ComPtr<ID3D11SamplerState> clampSampler;
	void PreRender();
	void PostRender();

	// Shadow map resources
	DirectX::XMFLOAT4X4 shadowViewMatrix;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> shadowSampler;
	
	int shadowMapResolution;
	float shadowProjectionSize;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> shadowDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowSRV;	
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> shadowRasterizer;
	DirectX::XMFLOAT4X4 shadowProjectionMatrix;

	int shadowMapResolution2;
	float shadowProjectionSize2;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> shadowDSV2;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowSRV2;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> shadowRasterizer2;
	DirectX::XMFLOAT4X4 shadowProjectionMatrix2;

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

