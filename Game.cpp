#include "Game.h"
#include "Vertex.h"
#include "Input.h"
#include "Exhibit.h"

// Needed for a helper function to read compiled shader files from the hard drive
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>
#include <math.h>

//For texture
#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,		   // The application's handle
		"DirectX Game",	   // Text for the window's title bar
		1280,			   // Width of the window's client area
		720,			   // Height of the window's client area
		true),			   // Show extra stats (fps) in title bar?
		shadowMapResolution(1024),
		shadowViewMatrix(),
		shadowProjectionMatrix(),
		shadowProjectionSize(10.0f)
{
	camera = 0;
	ambientColor= XMFLOAT3(0.1f, 0.1f, 0.1f);
	//Set up exhibit variables
	exhibitIndex = 0;
	brightness = 0.0f;
	contrast = 1.0f;
	blur = 0;

#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif

}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{

	for (auto& en : entityList) { 
		delete en; 
		en = nullptr;
	}

	for (auto& ex : exhibits) {
		delete ex;
		ex = nullptr;
	}

	for (auto& mat : materialList) {
		delete mat;
		mat = nullptr;
	}

	delete camera;
	camera = nullptr;

	delete cube;
	cube= nullptr;

	delete sphere;
	sphere = nullptr;

	delete pixelShader;
	pixelShader = nullptr;

	delete vertexShader;
	vertexShader = nullptr;

	delete pixelShaderSky;
	pixelShaderSky = nullptr;

	delete vertexShaderSky;
	vertexShaderSky = nullptr;

	delete pixelShaderSobel;
	pixelShaderSobel = nullptr;

	delete pixelShaderBrightCont;
	pixelShaderBrightCont = nullptr;

	delete pixelShaderBlur;
	pixelShaderBlur = nullptr;

	delete pixelShaderBloomE;
	pixelShaderBloomE = nullptr;

	delete vertexShaderFull;
	vertexShaderFull = nullptr;

	delete vertexShaderShadow;
	vertexShaderShadow = nullptr;

	delete sky;
	sky = nullptr;
}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateBasicGeometry();
	ResizePostProcessResources();
	D3D11_SAMPLER_DESC ppSampDesc = {};
	ppSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	ppSampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&ppSampDesc, ppSampler.GetAddressOf());
	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	CreateShadowMapResources();
	// Set up sprite batch and sprite font
	spriteBatch = std::make_unique<SpriteBatch>(context.Get());

	//Textures
	//create and zero out a local sampler desc variable
	D3D11_SAMPLER_DESC samplerDesc = {};
	//required aaaaa
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	//different filter modes
	//samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	//samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 5;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	//create sampler state
	device->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf());
	
	//Load textures
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/defaults/blackTexture.png").c_str(), 0, defaultBlackSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/defaults/defaultNormals.png").c_str(), 0, defaultNormalSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/defaults/pureWhite.png").c_str(), 0, pureWhiteSRV.GetAddressOf());
	
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> spaceBox = CreateCubemap(
		GetFullPathTo_Wide(L"../../Assets/Textures/SmallerSpaceBox/Left_Tex.png").c_str(),
		GetFullPathTo_Wide(L"../../Assets/Textures/SmallerSpaceBox/Right_Tex.png").c_str(),
		GetFullPathTo_Wide(L"../../Assets/Textures/SmallerSpaceBox/Up_Tex.png").c_str(),
		GetFullPathTo_Wide(L"../../Assets/Textures/SmallerSpaceBox/Down_Tex.png").c_str(),
		GetFullPathTo_Wide(L"../../Assets/Textures/SmallerSpaceBox/Front_Tex.png").c_str(),
		GetFullPathTo_Wide(L"../../Assets/Textures/SmallerSpaceBox/Back_Tex.png").c_str()
	);

	//create sky
	sky = new Sky(
		samplerState,
		spaceBox,
		cube,
		pixelShaderSky,
		vertexShaderSky,
		device,
		context);

	//create materials
	Material* material1 = CreateMaterial(
		L"../../Assets/Textures/earth_albedo.jpg",
		L"../../Assets/Textures/earth_normal.jpg",
		L"../../Assets/Textures/earth_roughness.jpg",
		nullptr
	);

	Material* material2 = CreateMaterial(
		L"../../Assets/Textures/moon_albedo.jpg",
		L"../../Assets/Textures/moon_normal.jpg",
		L"../../Assets/Textures/moon_roughness.jpg",
		nullptr
	);

	Material* material3 = CreateMaterial(
		L"../../Assets/Textures/cobblestone/cobblestone_albedo.png",
		L"../../Assets/Textures/cobblestone/cobblestone_normal.png",
		L"../../Assets/Textures/cobblestone/cobblestone_roughness.png",
		L"../../Assets/Textures/cobblestone/cobblestone_metal.png"
	);

	//create camera 
	camera = new Camera(0, 0, -10, 15.0f, 0.2f, XM_PIDIV4, (float)width / height);

	//default normal and uv values
	defNormal = XMFLOAT3(+0.0f, +0.0f, -1.0f);
	defUV = XMFLOAT2(+0.0f, +0.0f);

	//Sunlight from the left
	Light directionalLight1 = {};
	directionalLight1.Type = 0;
	directionalLight1.Direction= XMFLOAT3(1.0f, 0.0f, 0.0f);
	directionalLight1.Color= XMFLOAT3(0.1f, 0.1f, 0.1f);
	directionalLight1.Intensity = 5.0f;
	directionalLight1.CastsShadows = 1;
	//add entities to the entitiy list
	lightList.push_back(directionalLight1);

	firstPerson = true;
	Input::GetInstance().SwapMouseVisible();

	// set up exhibits
	Exhibit::cube = cube;
	Exhibit::cobblestone = material3;
	Exhibit::marble = CreateMaterial(
		L"../../Assets/Textures/marble/Marble_Tiles_001_basecolor.jpg",
		L"../../Assets/Textures/marble/Marble_Tiles_001_normal.jpg",
		L"../../Assets/Textures/marble/Marble_Tiles_001_roughness.jpg",
		nullptr
	);

	// intro exhibit
	exhibits[Intro] = new Exhibit(30);
	GameEntity* firstPillar = new GameEntity(cube, Exhibit::marble);
	entityList.push_back(firstPillar);
	firstPillar->GetTransform()->SetScale(5, 10, 5);
	exhibits[Intro]->PlaceObject(firstPillar, XMFLOAT3(0, 5, 0));

	GameEntity* introSign = new GameEntity(cube, 
		CreateMaterial(L"../../Assets/Textures/signs/intro sign.png", nullptr, nullptr, nullptr)
	);
	entityList.push_back(introSign);
	introSign->GetTransform()->SetScale(0.1f, 3.5f, 4.5f);
	introSign->GetTransform()->SetRotation(0, XM_PIDIV2, 0);
	exhibits[Intro]->PlaceObject(introSign, XMFLOAT3(0, 6, -2.5f));

	// brightness contrast exhibit
	exhibits[BrightContrast] = new Exhibit(55);
	exhibits[BrightContrast]->AttachTo(exhibits[Intro], NEGX);

	for (int i = 0; i < 100; i++) {
		GameEntity* colorSphere = new GameEntity(sphere, CreateColorMaterial(XMFLOAT3(2.55f * rand() / RAND_MAX, 2.55f * rand() / RAND_MAX, 2.55f * rand() / RAND_MAX)));
		entityList.push_back(colorSphere);
		exhibits[BrightContrast]->PlaceObject(colorSphere, XMFLOAT3(rand() % 24 - 12, 1 + rand() % 20, rand() % 24 - 12));
	}

	// blur exhibit
	exhibits[Blur] = new Exhibit(55);
	exhibits[Blur]->AttachTo(exhibits[Intro], POSX);

	Material* monaLisaMaterial = CreateMaterial(L"../../Assets/Textures/mona lisa.png", nullptr, nullptr, nullptr);
	Material* starryNightMaterial = CreateMaterial(L"../../Assets/Textures/starry night.jpg", nullptr, nullptr, nullptr);
	//Material* persistMemMaterial = CreateMaterial(L"../../Assets/Textures/persistence memory.png", nullptr, nullptr, nullptr);

	GameEntity* theMonaLisa = new GameEntity(cube, monaLisaMaterial);
	entityList.push_back(theMonaLisa);
	theMonaLisa->GetTransform()->SetScale(1.0f, 8.0f, 6.0f);
	theMonaLisa->GetTransform()->SetRotation(0.0f, XM_PIDIV2, 0.0f);
	exhibits[Blur]->PlaceObject(theMonaLisa, XMFLOAT3(0, 5.0f, -9.9f));

	GameEntity* starryNight = new GameEntity(cube, starryNightMaterial);
	entityList.push_back(starryNight);
	starryNight->GetTransform()->SetScale(1.0f, 7.0f, 10.0f);
	starryNight->GetTransform()->SetRotation(0.0f, XM_PIDIV2, 0.0f);
	exhibits[Blur]->PlaceObject(starryNight, XMFLOAT3(0, 5.0f, 9.9f));

	ditherObjects.push_back(theMonaLisa);
	ditherObjects.push_back(starryNight);

	/*GameEntity* persistenceMemory = new GameEntity(cube, persistMemMaterial);
	entityList.push_back(persistenceMemory);
	persistenceMemory->GetTransform()->SetScale(4.0f, 6.0f, 4.0f);
	persistenceMemory->GetTransform()->SetRotation(0.0f, XM_PIDIV2, 0.0f);
	exhibits[1]->PlaceObject(persistenceMemory, XMFLOAT3(-5, 3.0f, 0));*/

	// halls
	exhibits[LeftHall] = new Exhibit(20);
	exhibits[LeftHall]->AttachTo(exhibits[BrightContrast], POSZ);

	exhibits[RightHall] = new Exhibit(20);
	exhibits[RightHall]->AttachTo(exhibits[Blur], POSZ);

	// cel shading exhibit
	exhibits[CelShading] = new Exhibit(40);
	exhibits[CelShading]->AttachTo(exhibits[LeftHall], POSZ);

	Mesh* statueMesh = new Mesh(GetFullPathTo("../../Assets/Models/statue/statue.obj").c_str(), device);
	Material* statueMaterial = CreateColorMaterial(XMFLOAT3(0.5f, 0.5f, 0.5f));
	GameEntity* statue = new GameEntity(statueMesh, statueMaterial);
	statue->GetTransform()->SetScale(0.1f, 0.1f, 0.1f);
	statue->GetTransform()->SetRotation(XM_PIDIV2, -XM_PIDIV2, 0);
	entityList.push_back(statue);
	exhibits[CelShading]->PlaceObject(statue, XMFLOAT3(0.0f, 0.0f, 0.0f));

	// bloom & emmisive
	exhibits[Bloom] = new Exhibit(40);
	exhibits[Bloom]->AttachTo(exhibits[RightHall], POSZ);

	GameEntity* bloomSphere = new GameEntity(sphere, CreateColorMaterial(XMFLOAT3(2.55f, 0.0f, 2.55f)));
	entityList.push_back(bloomSphere);
	exhibits[Bloom]->PlaceObject(bloomSphere, XMFLOAT3(1.0f, 5.0f, 0.0f));

	// particles
	exhibits[Particles] = new Exhibit(45);
	exhibits[Particles]->AttachTo(exhibits[CelShading], POSX);
	exhibits[Particles]->AttachTo(exhibits[Bloom], NEGX);

	// final exhibit
	exhibits[Everything] = new Exhibit(70);
	exhibits[Everything]->AttachTo(exhibits[Particles], POSZ);

	/*GameEntity* earth = new GameEntity(sphere, material1);
	entityList.push_back(earth);
	earth->GetTransform()->SetPosition(0.0f, 0.0f, 0.0f);
	earth->GetTransform()->SetScale(1.0f, 1.0f, 1.0f);

	GameEntity* moon = new GameEntity(sphere, material2);
	entityList.push_back(moon);
	moon->GetTransform()->SetPosition(1.0f, 0.0f, 0.0f);
	moon->GetTransform()->SetScale(0.25f, 0.25f, 0.25f);*/
	//exhibits[0]->PlaceObject(entityList[0], DirectX::XMFLOAT3(0, 3, 0));
}

void Game::CreateShadowMapResources()
{
	// Create shadow requirements ------------------------------------------
	shadowMapResolution = 1024;
	shadowProjectionSize = 50.0f;

	// Create the actual texture that will be the shadow map
	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = shadowMapResolution;
	shadowDesc.Height = shadowMapResolution;
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowTexture;
	device->CreateTexture2D(&shadowDesc, 0, shadowTexture.GetAddressOf());

	// Create the depth/stencil
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSDesc = {};
	shadowDSDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(shadowTexture.Get(), &shadowDSDesc, shadowDSV.GetAddressOf());

	// Create the SRV for the shadow map
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(shadowTexture.Get(), &srvDesc, shadowSRV.GetAddressOf());

	// Create the special "comparison" sampler state for shadows
	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR; // COMPARISON filter!
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f;
	shadowSampDesc.BorderColor[1] = 1.0f;
	shadowSampDesc.BorderColor[2] = 1.0f;
	shadowSampDesc.BorderColor[3] = 1.0f;
	device->CreateSamplerState(&shadowSampDesc, &shadowSampler);

	// Create a rasterizer state
	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000; // Multiplied by (smallest possible positive value storable in the depth buffer)
	shadowRastDesc.DepthBiasClamp = 0.0f;
	shadowRastDesc.SlopeScaledDepthBias = 1.0f;
	device->CreateRasterizerState(&shadowRastDesc, &shadowRasterizer);

	// Create the "camera" matrices for the shadow map rendering

	// View
	XMMATRIX shView = XMMatrixLookAtLH(
		XMVectorSet(-5, 0, 0, 0),
		XMVectorSet(5, 0, 0, 0),
		XMVectorSet(0, 1, 0, 0));
	XMStoreFloat4x4(&shadowViewMatrix, shView);

	// Projection - we want ORTHOGRAPHIC for directional light shadows
	// NOTE: This particular projection is set up to be SMALLER than
	// the overall "scene", to show what happens when objects go
	// outside the shadow area.  In a game, you'd never want the
	// user to see this edge, but I'm specifically making the projection
	// small in this demo to show you that it CAN happen.
	//
	// Ideally, the first two parameters below would be adjusted to
	// fit the scene (or however much of the scene the user can see
	// at a time).  More advanced techniques, like cascaded shadow maps,
	// would use multiple (usually 4) shadow maps with increasingly larger
	// projections to ensure large open world games have shadows "everywhere"
	XMMATRIX shProj = XMMatrixOrthographicLH(shadowProjectionSize, shadowProjectionSize, 0.1f, 100.0f);
	XMStoreFloat4x4(&shadowProjectionMatrix, shProj);

}
// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	vertexShader = new SimpleVertexShader(device.Get(), context.Get(), GetFullPathTo_Wide(L"VertexShader.cso").c_str()); 
	pixelShader = new SimplePixelShader(device.Get(), context.Get(), GetFullPathTo_Wide(L"PixelShader.cso").c_str());
	vertexShaderShadow = new SimpleVertexShader(device.Get(), context.Get(), GetFullPathTo_Wide(L"VertexShaderShadow.cso").c_str());
	vertexShaderSky = new SimpleVertexShader(device.Get(), context.Get(), GetFullPathTo_Wide(L"VertexShaderSky.cso").c_str());
	pixelShaderSky = new SimplePixelShader(device.Get(), context.Get(), GetFullPathTo_Wide(L"PixelShaderSky.cso").c_str());
	vertexShaderFull = new SimpleVertexShader(device.Get(), context.Get(), GetFullPathTo_Wide(L"VertexShaderFull.cso").c_str());
	pixelShaderSobel = new SimplePixelShader(device.Get(), context.Get(), GetFullPathTo_Wide(L"PixelShaderSobel.cso").c_str());
	pixelShaderBrightCont = new SimplePixelShader(device.Get(), context.Get(), GetFullPathTo_Wide(L"PixelShaderBrightCont.cso").c_str());
	pixelShaderBlur = new SimplePixelShader(device.Get(), context.Get(), GetFullPathTo_Wide(L"PixelShaderBlur.cso").c_str());
	pixelShaderBloomE= new SimplePixelShader(device.Get(), context.Get(), GetFullPathTo_Wide(L"PixelShaderBloomE.cso").c_str());
	pixelShaderNoPostProcess = new SimplePixelShader(device.Get(), context.Get(), GetFullPathTo_Wide(L"PixelShaderNoPostProcess.cso").c_str());
}

void Game::CreateBasicGeometry()
{
	cube = new Mesh(GetFullPathTo("../../Assets/Models/cube.obj").c_str(), device);
	sphere = new Mesh(GetFullPathTo("../../Assets/Models/sphere.obj").c_str(), device);
}


// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();
	if (camera)
	{
		camera->UpdateProjectionMatrix((float)width / height);
	}
}

void Game::ResizePostProcessResources()
{
	// Reset all resources 
	ppRTV.Reset();
	pp2RTV.Reset();
	ppSRV.Reset();
	pp2SRV.Reset();
	sceneNormalsRTV.Reset();
	sceneNormalsSRV.Reset();
	sceneDepthRTV.Reset();
	sceneDepthSRV.Reset();

	// Describe our textures
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; 
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;

	// Create the color and normals textures
	Microsoft::WRL::ComPtr<ID3D11Texture2D> ppTexture;
	device->CreateTexture2D(&textureDesc, 0, ppTexture.GetAddressOf());

	Microsoft::WRL::ComPtr<ID3D11Texture2D> pp2Texture;
	device->CreateTexture2D(&textureDesc, 0, pp2Texture.GetAddressOf());

	// Adjust the description for scene normals
	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> sceneNormalsTexture;
	device->CreateTexture2D(&textureDesc, 0, sceneNormalsTexture.GetAddressOf());

	// Adjust the description for the scene depths
	textureDesc.Format = DXGI_FORMAT_R32_FLOAT;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> sceneDepthsTexture;
	device->CreateTexture2D(&textureDesc, 0, sceneDepthsTexture.GetAddressOf());

	// Create the Render Target Views (null descriptions use default settings)
	device->CreateRenderTargetView(ppTexture.Get(), 0, ppRTV.GetAddressOf());
	device->CreateRenderTargetView(pp2Texture.Get(), 0, pp2RTV.GetAddressOf());
	device->CreateRenderTargetView(sceneNormalsTexture.Get(), 0, sceneNormalsRTV.GetAddressOf());
	device->CreateRenderTargetView(sceneDepthsTexture.Get(), 0, sceneDepthRTV.GetAddressOf());

	// Create the Shader Resource Views (null descriptions use default settings)
	device->CreateShaderResourceView(ppTexture.Get(), 0, ppSRV.GetAddressOf());
	device->CreateShaderResourceView(pp2Texture.Get(), 0, pp2SRV.GetAddressOf());
	device->CreateShaderResourceView(sceneNormalsTexture.Get(), 0, sceneNormalsSRV.GetAddressOf());
	device->CreateShaderResourceView(sceneDepthsTexture.Get(), 0, sceneDepthSRV.GetAddressOf());
}


Material* Game::CreateMaterial(const wchar_t* albedoPath, const wchar_t* normalsPath, const wchar_t* roughnessPath, const wchar_t* metalPath)
{
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> albedoSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughnessSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> metalSRV;

	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(albedoPath).c_str(), 0, albedoSRV.GetAddressOf());
	
	if (normalsPath == nullptr) {
		normalSRV = defaultNormalSRV;
	} else {
		CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(normalsPath).c_str(), 0, normalSRV.GetAddressOf());
	}

	if (roughnessPath == nullptr) {
		roughnessSRV = defaultBlackSRV;
	} else {
		CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(roughnessPath).c_str(), 0, roughnessSRV.GetAddressOf());
	}

	if (metalPath == nullptr) {
		metalSRV = defaultBlackSRV;
	} else {
		CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(metalPath).c_str(), 0, metalSRV.GetAddressOf());
	}

	Material* material = new Material(DirectX::XMFLOAT3(+2.5f, +2.5f, +2.5f), 0.0f, pixelShader, vertexShader);
	material->AddSamplerState("BasicSamplerState", samplerState);
	material->AddTextureSRV("Albedo", albedoSRV);
	material->AddTextureSRV("NormalMap", normalSRV);
	material->AddTextureSRV("RoughnessMap", roughnessSRV);
	material->AddTextureSRV("MetalnessMap", metalSRV);
	materialList.push_back(material);
	return material;
}

Material* Game::CreateColorMaterial(XMFLOAT3 color)
{
	Material* material = new Material(color, 0.0f, pixelShader, vertexShader);
	material->AddSamplerState("BasicSamplerState", samplerState);
	material->AddTextureSRV("Albedo", pureWhiteSRV);
	material->AddTextureSRV("NormalMap", defaultNormalSRV);
	material->AddTextureSRV("RoughnessMap", defaultBlackSRV);
	material->AddTextureSRV("MetalnessMap", defaultBlackSRV);
	materialList.push_back(material);
	return material;
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Example input checking: Quit if the escape key is pressed
	if (Input::GetInstance().KeyDown(VK_ESCAPE))
		Quit();

	// allow exhibit walls to trap the camera
	for(Exhibit* exhibit : exhibits) {
		exhibit->CheckCollisions(camera);
	}

	// check for change in exhibit
	for (int i = 0; i < NUM_EXHIBITS; i++) {
		if (i == exhibitIndex) {
			continue;
		}

		if (exhibits[i]->IsInExhibit(camera->GetTransform()->GetPosition())) {
			exhibitIndex = i;

			// reset values when leaving a room
			numCels = 0;
			brightness = 0.0f;
			contrast = 1.0f;
			break;
		}
	}

	//make items rotate along y axis
	/*static float increment = 0.5f;
	increment += 0.0005f;*/
	
	// move moon
	/*entityList[0]->GetTransform()->SetRotation(0.0f, increment/2, 0.0f);
	entityList[1]->GetTransform()->SetPosition(2*sin(increment), 0.0f, 2*cos(increment));
	exhibits[0]->PlaceObject(entityList[1], DirectX::XMFLOAT3(2 * sin(increment), 3.0f, 2 * cos(increment)))*/;

	//code that will alter fov based on user input
	/*float fov = camera->GetFoV();
	if (Input::GetInstance().KeyDown('P')) {
		fov += 1 * deltaTime;
		camera->SetFoV(fov);
	}
	if (Input::GetInstance().KeyDown('O')) {
		fov -= 1 * deltaTime;
		camera->SetFoV(fov);
	}*/

	if (Input::GetInstance().KeyPress('E')) {
		firstPerson = !firstPerson;
		Input::GetInstance().SwapMouseVisible();
	}
	if (firstPerson) {
		camera->Update(deltaTime);
	}
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	PreRender();
	//

	//call game entity drawing method for each entity
	//also set up lighting stuff
	
	for (int i = 0; i < entityList.size(); i++) {
		entityList[i]->GetMaterial()->GetVertexShader()->SetMatrix4x4("shadowView", shadowViewMatrix);
		entityList[i]->GetMaterial()->GetVertexShader()->SetMatrix4x4("shadowProjection", shadowProjectionMatrix);
		
		entityList[i]->GetMaterial()->GetPixelShader()->SetFloat3("ambientColor",ambientColor);
		entityList[i]->GetMaterial()->GetPixelShader()->SetData("lights", &lightList[0], sizeof(Light) * (int)lightList.size());
		entityList[i]->GetMaterial()->GetPixelShader()->SetShaderResourceView("ShadowMap", shadowSRV);
		entityList[i]->GetMaterial()->GetPixelShader()->SetSamplerState("ShadowSampler", shadowSampler);
		entityList[i]->GetMaterial()->GetPixelShader()->SetInt("numCels", numCels);
		entityList[i]->GetMaterial()->GetPixelShader()->SetFloat("transparency", entityList[i]->GetMaterial()->GetTransparency());
		entityList[i]->Draw(context,camera);
	}

	// draw all exhibits
	for (Exhibit* exhibit : exhibits) {
		const std::vector<GameEntity*>* surfaces = exhibit->GetEntities();
		for (GameEntity* surface : *surfaces) {
			surface->GetMaterial()->GetVertexShader()->SetMatrix4x4("shadowView", shadowViewMatrix);
			surface->GetMaterial()->GetVertexShader()->SetMatrix4x4("shadowProjection", shadowProjectionMatrix);

			surface->GetMaterial()->GetPixelShader()->SetFloat3("ambientColor", ambientColor);
			surface->GetMaterial()->GetPixelShader()->SetData("lights", &lightList[0], sizeof(Light) * (int)lightList.size());
			surface->GetMaterial()->GetPixelShader()->SetInt("numCels", numCels);
			surface->GetMaterial()->GetPixelShader()->SetShaderResourceView("ShadowMap", shadowSRV);
			surface->GetMaterial()->GetPixelShader()->SetSamplerState("ShadowSampler", shadowSampler);
			surface->GetMaterial()->GetPixelShader()->SetFloat("transparency", surface->GetMaterial()->GetTransparency());
			
			surface->Draw(context, camera);
		}
	}
	
	//draw sky
	sky->Draw(context, camera);
	
	//draw ImGui elements
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	//Create ImGui Window
	ImGui::Begin("Control Panel");
	ImGui::DragFloat(": sensitivity", &camera->mouseLookSpeed, 0.01f, 0.01f, 10.0f);
	
	if(exhibitIndex == BrightContrast || exhibitIndex == Everything) {
		ImGui::DragFloat(": brightness", &brightness, 0.01f, -1.0f, 1.0f);
		ImGui::DragFloat(": contrast", &contrast, 0.01f, 0.0f, 10.0f);
	}
	if(exhibitIndex == Blur || exhibitIndex == Everything) {
		ImGui::DragInt(": blur", &blur, 1, 0, 50);
		ImGui::DragFloat(": transparency", &transparency, 0.01, 0, 1);
		for (GameEntity* entity : ditherObjects) {
			entity->GetMaterial()->SetTransparency(transparency);
		}
	}
	if(exhibitIndex == CelShading || exhibitIndex == Everything) {
		ImGui::SliderInt(": cels", &numCels, 0, 6);
		ImGui::Checkbox(": outline", &useSobel);
	}
	if(exhibitIndex == Bloom || exhibitIndex == Everything) {
		ImGui::Checkbox(": bloom", &useBloom);
		ImGui::DragFloat(": bloom threshold", &bloomThreshold, 0.01f, 0.5f, 1.0f);
		ImGui::DragFloat(": bloom intensity", &bloomIntensity, 0.01f, 0.5f, 2.0f);
		ImGui::DragFloat(": bloom saturation", &bloomSaturation, 0.01f, 0.5f, 1.0f);
		ImGui::DragFloat(": bloom blur sigma", &bloomBlurSigma, 0.01f, 0.5f, 2.0f);
		ImGui::DragFloat(": bloom blur radius", &bloomBlurRadius, 0.01f, 1.0f, 7.0f);
		ImGui::DragFloat(": bloom blur step size", &bloomBlurStepSize, 0.01f, 0.0f, 2.0f);
	}

	ImGui::End();
	
	//unbind shadow map
	ID3D11ShaderResourceView* nullSRVs[16] = {};
	context->PSSetShaderResources(0, 16, nullSRVs);
	PostRender();
	ImGui::Render(); //Assemble Together Draw Data
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());//Render Draw Data
	swapChain->Present(0, 0);
	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthStencilView.Get());
}

void Game::PreRender()
{
	// Background color 
	const float color[4] = { 0, 0, 0, 1 };

	// Clear the render target and depth buffer
	context->ClearRenderTargetView(backBufferRTV.Get(), color);
	context->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	RenderShadowMap();
	// Clear all render targets
	context->ClearRenderTargetView(ppRTV.Get(), color);
	context->ClearRenderTargetView(pp2RTV.Get(), color);
	context->ClearRenderTargetView(sceneNormalsRTV.Get(), color);
	context->ClearRenderTargetView(sceneDepthRTV.Get(), color);

	ID3D11RenderTargetView* rtvs[3] =
	{
		backBufferRTV.Get(),
		sceneNormalsRTV.Get(),
		sceneDepthRTV.Get()
	};

	rtvs[0] = ppRTV.Get();
	context->OMSetRenderTargets(3, rtvs, depthStencilView.Get());
}

void Game::RenderShadowMap()
{
	// Initial pipeline setup - No RTV necessary - Clear shadow map
	context->OMSetRenderTargets(0, 0, shadowDSV.Get());
	context->ClearDepthStencilView(shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	context->RSSetState(shadowRasterizer.Get());
	
	// Need to create a viewport that matches the shadow map resolution
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = (float)shadowMapResolution;
	viewport.Height = (float)shadowMapResolution;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);

	// Turn on our shadow map Vertex Shader
	// and turn OFF the pixel shader entirely
	
	vertexShaderShadow->SetShader();
	vertexShaderShadow->SetMatrix4x4("view", shadowViewMatrix);
	vertexShaderShadow->SetMatrix4x4("projection", shadowProjectionMatrix);
	context->PSSetShader(0, 0, 0); // No PS

	// Loop and draw all entities
	for (auto& e : entityList)
	{
		vertexShaderShadow->SetMatrix4x4("world", e->GetTransform()->GetWorldMatrix());
		vertexShaderShadow->CopyAllBufferData();
		// Draw the mesh
		e->GetMesh()->Draw(context);
	}
	
	for (Exhibit* exhibit : exhibits) {
		const std::vector<GameEntity*>* surfaces = exhibit->GetEntities();
		for (GameEntity* surface : *surfaces) {
			vertexShaderShadow->SetMatrix4x4("world", surface->GetTransform()->GetWorldMatrix());
			vertexShaderShadow->CopyAllBufferData();
			// Draw the mesh
			surface->GetMesh()->Draw(context);
		}
	}
	// After rendering the shadow map, go back to the screen
	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthStencilView.Get());
	viewport.Width = (float)this->width;
	viewport.Height = (float)this->height;
	context->RSSetViewports(1, &viewport);
	context->RSSetState(0);
}

void Game::PostRender()
{
	vertexShaderFull->SetShader();

	// rendered screen starts in ppRTV
	std::vector<ExhbitType> postProcesses;

	// show that the system can work with any combination of post process shaders
	if(useBloom && (exhibitIndex == Bloom || exhibitIndex == Everything)) {
		postProcesses.push_back(Bloom);
	}
	if(exhibitIndex == BrightContrast || exhibitIndex == Everything) {
		postProcesses.push_back(BrightContrast);
	}
	if(exhibitIndex == Blur || exhibitIndex == Everything) {
		postProcesses.push_back(Blur);
	}
	if(useSobel && (exhibitIndex == CelShading || exhibitIndex == Everything)) {
		postProcesses.push_back(CelShading);
	}
	if (postProcesses.size() <= 0) {
		// use default post process if not using any
		postProcesses.push_back(Intro); // using Intro to represent no post processes
	}

	for (int i = 0; i < postProcesses.size(); i++) {
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> inputImage;
		if (i % 2 == 0) {
			// render every other to this render target
			inputImage = ppSRV;
			context->OMSetRenderTargets(1, pp2RTV.GetAddressOf(), 0);
		} else {
			inputImage = pp2SRV;
			context->OMSetRenderTargets(1, ppRTV.GetAddressOf(), 0);
		}

		// render the last one to the back buffer
		if (i == postProcesses.size() - 1) {
			context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), 0);
		}
		
		switch (postProcesses[i]) {
			case Intro:
				// no post processes
				pixelShaderNoPostProcess->SetShader();
				pixelShaderBrightCont->SetShaderResourceView("image", inputImage.Get());
				pixelShaderBrightCont->SetSamplerState("samplerOptions", clampSampler.Get());
				pixelShaderBrightCont->CopyAllBufferData();
				break;

			case BrightContrast:
				pixelShaderBrightCont->SetShader();
				pixelShaderBrightCont->SetShaderResourceView("image", inputImage.Get());
				pixelShaderBrightCont->SetSamplerState("samplerOptions", clampSampler.Get());
				pixelShaderBrightCont->SetFloat("brightness", brightness);
				pixelShaderBrightCont->SetFloat("contrast", contrast);
				pixelShaderBrightCont->CopyAllBufferData();
				break;

			case Blur:
				pixelShaderBlur->SetShader();
				pixelShaderBlur->SetShaderResourceView("image", inputImage.Get());
				pixelShaderBlur->SetSamplerState("samplerOptions", clampSampler.Get());
				pixelShaderBlur->SetInt("blur", blur);
				pixelShaderBlur->CopyAllBufferData();
				break;

			case CelShading: // sobel
				pixelShaderSobel->SetShader();
				pixelShaderSobel->SetShaderResourceView("image", inputImage.Get());
				pixelShaderSobel->SetSamplerState("samplerOptions", clampSampler.Get());
				pixelShaderSobel->SetFloat("pixelWidth", 1.0f / width);
				pixelShaderSobel->SetFloat("pixelHeight", 1.0f / height);
				pixelShaderSobel->CopyAllBufferData();
				break;

			case Bloom:
				pixelShaderBloomE->SetShader();
				pixelShaderBloomE->SetShaderResourceView("pixels", inputImage.Get()); // IMPORTANT: This step takes the original post process texture!
				pixelShaderBloomE->SetSamplerState("samplerOptions", clampSampler.Get());
				pixelShaderBloomE->SetFloat("bloomThreshold", bloomThreshold);
				pixelShaderBloomE->SetFloat("bloomIntensity", bloomIntensity);
				pixelShaderBloomE->SetFloat("bloomSaturation", bloomSaturation);
				pixelShaderBloomE->SetFloat("bloomBlurSigma", bloomBlurSigma);
				pixelShaderBloomE->SetFloat("bloomBlurRadius", bloomBlurRadius);
				pixelShaderBloomE->SetFloat("bloomBlurStepSize", bloomBlurStepSize);
				pixelShaderBloomE->SetFloat("pixelWidth", 1.0f / width);
				pixelShaderBloomE->SetFloat("pixelHeight", 1.0f / height);
				pixelShaderBloomE->CopyAllBufferData();
				break;
		}

		// draw the full screen triangle
		context->Draw(3, 0);
	}

	//if (exhibitIndex == Intro
	//	|| (exhibitIndex == CelShading && !useSobel)
	//	|| (exhibitIndex == Bloom && !useBloom)
	//) {
	//	// no post processes
	//	pixelShaderNoPostProcess->SetShader();
	//	pixelShaderBrightCont->SetShaderResourceView("image", ppSRV.Get());
	//	pixelShaderBrightCont->SetSamplerState("samplerOptions", clampSampler.Get());
	//	pixelShaderBrightCont->CopyAllBufferData();
	//}
	//if (exhibitIndex == BrightContrast) {
	//	pixelShaderBrightCont->SetShader();
	//	pixelShaderBrightCont->SetShaderResourceView("image", ppSRV.Get());
	//	pixelShaderBrightCont->SetSamplerState("samplerOptions", clampSampler.Get());
	//	pixelShaderBrightCont->SetFloat("brightness", brightness);
	//	pixelShaderBrightCont->SetFloat("contrast", contrast);
	//	pixelShaderBrightCont->CopyAllBufferData();
	//}
	//if (exhibitIndex == Blur) {
	//	pixelShaderBlur->SetShader();
	//	pixelShaderBlur->SetShaderResourceView("image", ppSRV.Get());
	//	pixelShaderBlur->SetSamplerState("samplerOptions", clampSampler.Get());
	//	pixelShaderBlur->SetInt("blur", blur);
	//	pixelShaderBlur->CopyAllBufferData();
	//}
	//if (exhibitIndex == CelShading && useSobel) {
	//	pixelShaderSobel->SetShader();
	//	pixelShaderSobel->SetShaderResourceView("image", ppSRV.Get());
	//	pixelShaderSobel->SetSamplerState("samplerOptions", clampSampler.Get());
	//	pixelShaderSobel->SetFloat("pixelWidth", 1.0f / width);
	//	pixelShaderSobel->SetFloat("pixelHeight", 1.0f / height);
	//	pixelShaderSobel->CopyAllBufferData();
	//}
	//if(exhibitIndex == Bloom && useBloom) {
	//	pixelShaderBloomE->SetShader();
	//	pixelShaderBloomE->SetShaderResourceView("pixels", ppSRV.Get()); // IMPORTANT: This step takes the original post process texture!
	//	pixelShaderBloomE->SetSamplerState("samplerOptions", clampSampler.Get());
	//	pixelShaderBloomE->SetFloat("bloomThreshold", bloomThreshold);
	//	pixelShaderBloomE->SetFloat("bloomIntensity", bloomIntensity);
	//	pixelShaderBloomE->SetFloat("bloomSaturation", bloomSaturation);
	//	pixelShaderBloomE->SetFloat("bloomBlurSigma", bloomBlurSigma);
	//	pixelShaderBloomE->SetFloat("bloomBlurRadius", bloomBlurRadius);
	//	pixelShaderBloomE->SetFloat("bloomBlurStepSize", bloomBlurStepSize);
	//	pixelShaderBloomE->SetFloat("pixelWidth", 1.0f / width);
	//	pixelShaderBloomE->SetFloat("pixelHeight", 1.0f / height);
	//	pixelShaderBloomE->CopyAllBufferData();
	//}

	// Draw 3 vertices
	//context->Draw(3, 0);
	// Unbind shader resource views at the end of the frame
	ID3D11ShaderResourceView* nullSRVs[128] = {};
	context->PSSetShaderResources(0, ARRAYSIZE(nullSRVs), nullSRVs);
}

// --------------------------------------------------------
// Loads six individual textures (the six faces of a cube map), then
// creates a blank cube map and copies each of the six textures to
// another face.  Afterwards, creates a shader resource view for
// the cube map and cleans up all of the temporary resources.
// --------------------------------------------------------
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Game::CreateCubemap(
	const wchar_t* right,
	const wchar_t* left,
	const wchar_t* up,
	const wchar_t* down,
	const wchar_t* front,
	const wchar_t* back)
{
	// Load the 6 textures into an array.
	// - We need references to the TEXTURES, not the SHADER RESOURCE VIEWS!
	// - Specifically NOT generating mipmaps, as we usually don't need them for the sky!
	// - Order matters here!  +X, -X, +Y, -Y, +Z, -Z
	ID3D11Texture2D* textures[6] = {};
	CreateWICTextureFromFile(device.Get(), right, (ID3D11Resource**)&textures[0], 0);
	CreateWICTextureFromFile(device.Get(), left, (ID3D11Resource**)&textures[1], 0);
	CreateWICTextureFromFile(device.Get(), up, (ID3D11Resource**)&textures[2], 0);
	CreateWICTextureFromFile(device.Get(), down, (ID3D11Resource**)&textures[3], 0);
	CreateWICTextureFromFile(device.Get(), front, (ID3D11Resource**)&textures[4], 0);
	CreateWICTextureFromFile(device.Get(), back, (ID3D11Resource**)&textures[5], 0);

	// We'll assume all of the textures are the same color format and resolution,
	// so get the description of the first shader resource view
	D3D11_TEXTURE2D_DESC faceDesc = {};
	textures[0]->GetDesc(&faceDesc);

	// Describe the resource for the cube map, which is simply 
	// a "texture 2d array".  This is a special GPU resource format, 
	// NOT just a C++ array of textures!!!
	D3D11_TEXTURE2D_DESC cubeDesc = {};
	cubeDesc.ArraySize = 6; // Cube map!
	cubeDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // We'll be using as a texture in a shader
	cubeDesc.CPUAccessFlags = 0; // No read back
	cubeDesc.Format = faceDesc.Format; // Match the loaded texture's color format
	cubeDesc.Width = faceDesc.Width;  // Match the size
	cubeDesc.Height = faceDesc.Height; // Match the size
	cubeDesc.MipLevels = 1; // Only need 1
	cubeDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE; // A CUBE MAP, not 6 separate textures
	cubeDesc.Usage = D3D11_USAGE_DEFAULT; // Standard usage
	cubeDesc.SampleDesc.Count = 1;
	cubeDesc.SampleDesc.Quality = 0;

	// Create the actual texture resource
	ID3D11Texture2D* cubeMapTexture = 0;
	device->CreateTexture2D(&cubeDesc, 0, &cubeMapTexture);

	// Loop through the individual face textures and copy them,
	// one at a time, to the cube map texure
	for (int i = 0; i < 6; i++)
	{
		// Calculate the subresource position to copy into
		unsigned int subresource = D3D11CalcSubresource(
			0,	// Which mip (zero, since there's only one)
			i,	// Which array element?
			1); 	// How many mip levels are in the texture?

		// Copy from one resource (texture) to another
		context->CopySubresourceRegion(
			cubeMapTexture, // Destination resource
			subresource,	// Dest subresource index (one of the array elements)
			0, 0, 0,		// XYZ location of copy
			textures[i],	// Source resource
			0,	// Source subresource index (we're assuming there's only one)
			0);	// Source subresource "box" of data to copy (zero means the whole thing)
	}

	// At this point, all of the faces have been copied into the 
	// cube map texture, so we can describe a shader resource view for it
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = cubeDesc.Format; 	// Same format as texture
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE; // Treat this as a cube!
	srvDesc.TextureCube.MipLevels = 1;	// Only need access to 1 mip
	srvDesc.TextureCube.MostDetailedMip = 0; // Index of the first mip we want to see

	// Make the SRV
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cubeSRV;
	device->CreateShaderResourceView(cubeMapTexture, &srvDesc, cubeSRV.GetAddressOf());

	// Now that we're done, clean up the stuff we don't need anymore
	cubeMapTexture->Release();  // Done with this particular reference (the SRV has another)
	for (int i = 0; i < 6; i++)
		textures[i]->Release();

	// Send back the SRV, which is what we need for our shaders
	return cubeSRV;
}
