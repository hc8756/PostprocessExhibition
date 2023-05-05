#include "Game.h"
#include "Vertex.h"
#include "Particle.h"
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
		true)			   // Show extra stats (fps) in title bar?
{
	camera = 0;
	ambientColor= XMFLOAT3(0.2f, 0.2f, 0.2f);
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

	delete pixelShaderParticle;
	pixelShaderParticle = nullptr;

	delete vertexShaderParticle;
	vertexShaderParticle = nullptr;

	delete sky;
	sky = nullptr;

	delete particleManager;
	particleManager = nullptr;
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
	CreateParticleStates();
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

	//create camera 
	camera = new Camera(0, 6, -10, 15.0f, 0.2f, XM_PIDIV4, (float)width / height);

	//default normal and uv values
	defNormal = XMFLOAT3(+0.0f, +0.0f, -1.0f);
	defUV = XMFLOAT2(+0.0f, +0.0f);

	// create directional light
	Light directionalLight1 = {};
	directionalLight1.Type = 0;
	directionalLight1.Direction = XMFLOAT3(0, -sin(XM_PI / 3), cos(XM_PI / 3)); // must match shadow map angle
	XMVECTOR direction = XMVector3Rotate(
		XMLoadFloat3(&directionalLight1.Direction),
		XMQuaternionRotationRollPitchYawFromVector(XMVectorSet(0, XM_PI / 6, 0, 0))
	);
	XMStoreFloat3(&directionalLight1.Direction, direction);

	directionalLight1.Color= XMFLOAT3(0.1f, 0.1f, 0.1f);
	directionalLight1.Intensity = 10.0f;
	directionalLight1.CastsShadows = 1;
	lightList.push_back(directionalLight1);

	firstPerson = true;
	Input::GetInstance().SwapMouseVisible();

	// set up exhibits
	Exhibit::cube = cube;
	Exhibit::cobblestone = CreateMaterial(
		L"../../Assets/Textures/cobblestone/cobblestone_albedo.png",
		L"../../Assets/Textures/cobblestone/cobblestone_normals.png",
		L"../../Assets/Textures/cobblestone/cobblestone_roughness.png",
		L"../../Assets/Textures/cobblestone/cobblestone_metal.png"
	);
	Exhibit::marble = CreateMaterial(
		L"../../Assets/Textures/marble/Marble_Tiles_001_basecolor.jpg",
		L"../../Assets/Textures/marble/Marble_Tiles_001_normal.jpg",
		L"../../Assets/Textures/marble/Marble_Tiles_001_roughness.jpg",
		nullptr
	);

	auto MakeSign = [&](Material* material) {
		GameEntity* sign = new GameEntity(cube, material);
		entityList.push_back(sign);
		sign->GetTransform()->SetScale(0.1f, 3.5f, 4.5f);
		return sign;
	};

	Material* blurSignMat = CreateMaterial(L"../../Assets/Textures/signs/blur sign.png", nullptr, nullptr, nullptr);
	Material* brightnessSignMat = CreateMaterial(L"../../Assets/Textures/signs/brightness sign.png", nullptr, nullptr, nullptr);
	Material* bloomSignMat = CreateMaterial(L"../../Assets/Textures/signs/bloom sign.png", nullptr, nullptr, nullptr);
	Material* celSignMat = CreateMaterial(L"../../Assets/Textures/signs/cel sign.png", nullptr, nullptr, nullptr);
	Material* particleSignMat = CreateMaterial(L"../../Assets/Textures/signs/particle sign.png", nullptr, nullptr, nullptr);

	// intro exhibit
	exhibits[Intro] = new Exhibit(30);
	GameEntity* firstPillar = new GameEntity(cube, Exhibit::marble);
	entityList.push_back(firstPillar);
	firstPillar->GetTransform()->SetScale(5, 10, 5);
	exhibits[Intro]->PlaceObject(firstPillar, XMFLOAT3(0, 5, 0));

	GameEntity* introSign = MakeSign(CreateMaterial(L"../../Assets/Textures/signs/intro sign.png", nullptr, nullptr, nullptr));
	introSign->GetTransform()->SetRotation(0, XM_PIDIV2, 0);
	exhibits[Intro]->PlaceObject(introSign, XMFLOAT3(0, 6, -2.5f));

	GameEntity* introToBlurSign = MakeSign(blurSignMat);
	exhibits[Intro]->PlaceObject(introToBlurSign, XMFLOAT3(14.5f, 6, -7.0f));

	GameEntity* introToBrightnessSign = MakeSign(brightnessSignMat);
	exhibits[Intro]->PlaceObject(introToBrightnessSign, XMFLOAT3(-14.5f, 6, -7.0f));

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
	Material* persistMemMaterial = CreateMaterial(L"../../Assets/Textures/persistence memory.png", nullptr, nullptr, nullptr);
	Material* pearlEarringMat = CreateMaterial(L"../../Assets/Textures/girl pearl earring.jpg", nullptr, nullptr, nullptr);
	Material* convergenceMat = CreateMaterial(L"../../Assets/Textures/convergence.jpg", nullptr, nullptr, nullptr);
	Material* sundayAfternoonMat = CreateMaterial(L"../../Assets/Textures/sunday afternoon.jpg", nullptr, nullptr, nullptr);
	Material* impressionSunriseMat = CreateMaterial(L"../../Assets/Textures/impression sunrise.jpg", nullptr, nullptr, nullptr);
	// neon lights
	Material* neonlight1 = CreateMaterial(L"../../Assets/Textures/neon/neonlight1.png", nullptr, nullptr, nullptr);
	Material* neonlight2 = CreateMaterial(L"../../Assets/Textures/neon/neonlight2.png", nullptr, nullptr, nullptr);
	Material* neonlight3 = CreateMaterial(L"../../Assets/Textures/neon/neonlight3.png", nullptr, nullptr, nullptr);

	GameEntity* starryNight = new GameEntity(cube, starryNightMaterial);
	entityList.push_back(starryNight);
	starryNight->GetTransform()->SetScale(1.0f, 12.0f, 16.0f);
	starryNight->GetTransform()->SetRotation(0.0f, XM_PIDIV2, 0.0f);
	exhibits[Blur]->PlaceObject(starryNight, XMFLOAT3(-16.0f, 7.5f, 27.4f));

	GameEntity* persistenceMemory = new GameEntity(cube, persistMemMaterial);
	entityList.push_back(persistenceMemory);
	persistenceMemory->GetTransform()->SetScale(1.0f, 12.0f, 16.0f);
	persistenceMemory->GetTransform()->SetRotation(0.0f, XM_PIDIV2, 0.0f);
	exhibits[Blur]->PlaceObject(persistenceMemory, XMFLOAT3(16.0f, 7.5f, 27.4f));

	GameEntity* theMonaLisa = new GameEntity(cube, monaLisaMaterial);
	entityList.push_back(theMonaLisa);
	theMonaLisa->GetTransform()->SetScale(1.0f, 12.0f, 9.0);
	exhibits[Blur]->PlaceObject(theMonaLisa, XMFLOAT3(-27.4f, 7.5f, 16.0f));

	GameEntity* pearlEarring = new GameEntity(cube, pearlEarringMat);
	entityList.push_back(pearlEarring);
	pearlEarring->GetTransform()->SetScale(1.0f, 12.0f, 9.0f);
	exhibits[Blur]->PlaceObject(pearlEarring, XMFLOAT3(-27.4f, 7.5f, -16.0f));

	GameEntity* convergence = new GameEntity(cube, convergenceMat);
	entityList.push_back(convergence);
	convergence->GetTransform()->SetScale(1.0f, 12.0f, 20.0f);
	exhibits[Blur]->PlaceObject(convergence, XMFLOAT3(27.4f, 7.5f, 0.0f));

	GameEntity* sundayAfternoon = new GameEntity(cube, sundayAfternoonMat);
	entityList.push_back(sundayAfternoon);
	sundayAfternoon->GetTransform()->SetScale(1.0f, 12.0f, 16.0f);
	sundayAfternoon->GetTransform()->SetRotation(0.0f, XM_PIDIV2, 0.0f);
	exhibits[Blur]->PlaceObject(sundayAfternoon, XMFLOAT3(-12.0f, 7.5f, -27.4f));

	GameEntity* impressionSunrise = new GameEntity(cube, impressionSunriseMat);
	entityList.push_back(impressionSunrise);
	impressionSunrise->GetTransform()->SetScale(1.0f, 12.0f, 16.0f);
	impressionSunrise->GetTransform()->SetRotation(0.0f, XM_PIDIV2, 0.0f);
	exhibits[Blur]->PlaceObject(impressionSunrise, XMFLOAT3(12.0f, 7.5f, -27.4f));

	ditherObjects.push_back(theMonaLisa);
	ditherObjects.push_back(starryNight);
	ditherObjects.push_back(persistenceMemory);
	ditherObjects.push_back(pearlEarring);
	ditherObjects.push_back(convergence);
	ditherObjects.push_back(sundayAfternoon);
	ditherObjects.push_back(impressionSunrise);
	
	// halls
	exhibits[LeftHall] = new Exhibit(20);
	exhibits[LeftHall]->AttachTo(exhibits[BrightContrast], POSZ);
	GameEntity* hallToBright = MakeSign(brightnessSignMat);
	hallToBright->GetTransform()->SetRotation(0, XM_PIDIV2, 0);
	exhibits[LeftHall]->PlaceObject(hallToBright, XMFLOAT3(-6.5f, 7.5f, -9.5f));
	GameEntity* hallToCel = MakeSign(celSignMat);
	hallToCel->GetTransform()->SetRotation(0, XM_PIDIV2, 0);
	exhibits[LeftHall]->PlaceObject(hallToCel, XMFLOAT3(-6.5f, 7.5f, 9.5f));

	exhibits[RightHall] = new Exhibit(20);
	exhibits[RightHall]->AttachTo(exhibits[Blur], POSZ);
	GameEntity* hallToBlur = MakeSign(blurSignMat);
	hallToBlur->GetTransform()->SetRotation(0, XM_PIDIV2, 0);
	exhibits[RightHall]->PlaceObject(hallToBlur, XMFLOAT3(6.5f, 7.5f, -9.5f));
	GameEntity* hallToBloom = MakeSign(bloomSignMat);
	hallToBloom->GetTransform()->SetRotation(0, XM_PIDIV2, 0);
	exhibits[RightHall]->PlaceObject(hallToBloom, XMFLOAT3(6.5f, 7.5f, 9.5f));

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
	GameEntity* celToParticle = MakeSign(particleSignMat);
	exhibits[CelShading]->PlaceObject(celToParticle, XMFLOAT3(19.5f, 7.5f, 7.0f));

	// bloom & emmisive
	exhibits[Bloom] = new Exhibit(40);
	exhibits[Bloom]->AttachTo(exhibits[RightHall], POSZ);
	GameEntity* neonlightObj1 = new GameEntity(cube, neonlight1);
	GameEntity* neonlightObj4 = new GameEntity(cube, neonlight1);
	entityList.push_back(neonlightObj1);
	entityList.push_back(neonlightObj4);
	neonlightObj1->GetTransform()->SetScale(1.0f, 12.0f, 12.0f);
	neonlightObj4->GetTransform()->SetScale(1.0f, 12.0f, 12.0f);
	neonlightObj1->GetTransform()->SetRotation(0.0f, XM_PIDIV2, 0.0f);
	neonlightObj4->GetTransform()->SetRotation(0.0f, XM_PI, 0.0f);
	exhibits[Bloom]->PlaceObject(neonlightObj1, XMFLOAT3(10, 7, 19.5));
	exhibits[Bloom]->PlaceObject(neonlightObj4, XMFLOAT3(19.5, 7, 10));

	GameEntity* neonlightObj2 = new GameEntity(cube, neonlight2);
	GameEntity* neonlightObj5 = new GameEntity(cube, neonlight2);
	entityList.push_back(neonlightObj2);
	entityList.push_back(neonlightObj5);
	neonlightObj2->GetTransform()->SetScale(1.0f, 12.0f, 12.0f);
	neonlightObj5->GetTransform()->SetScale(1.0f, 12.0f, 12.0f);
	neonlightObj2->GetTransform()->SetRotation(0.0f, XM_PIDIV2, 0.0f);
	neonlightObj5->GetTransform()->SetRotation(0.0f, XM_PI, 0.0f);
	exhibits[Bloom]->PlaceObject(neonlightObj2, XMFLOAT3(0, 7, 19.5));
	exhibits[Bloom]->PlaceObject(neonlightObj5, XMFLOAT3(19.5, 7, 0));

	GameEntity* neonlightObj3 = new GameEntity(cube, neonlight3);
	GameEntity* neonlightObj6 = new GameEntity(cube, neonlight3);
	entityList.push_back(neonlightObj3);
	entityList.push_back(neonlightObj6);
	neonlightObj3->GetTransform()->SetScale(1.0f, 12.0f, 12.0f);
	neonlightObj6->GetTransform()->SetScale(1.0f, 12.0f, 12.0f);
	neonlightObj3->GetTransform()->SetRotation(0.0f, XM_PIDIV2, 0.0f);
	neonlightObj6->GetTransform()->SetRotation(0.0f, XM_PI, 0.0f);
	exhibits[Bloom]->PlaceObject(neonlightObj3, XMFLOAT3(-10, 7, 19.5));
	exhibits[Bloom]->PlaceObject(neonlightObj6, XMFLOAT3(19.5, 7, -10));
	GameEntity* bloomToParticle = MakeSign(particleSignMat);
	exhibits[Bloom]->PlaceObject(bloomToParticle, XMFLOAT3(-19.5f, 7.5f, 7.0f));

	// particles
	exhibits[Particles] = new Exhibit(45);
	exhibits[Particles]->AttachTo(exhibits[CelShading], POSX);
	exhibits[Particles]->AttachTo(exhibits[Bloom], NEGX);
	XMFLOAT3 pmStartPos = exhibits[Particles]->origin;
	pmStartPos.x /= 2; // adjust position
	pmStartPos.z /= 2;
	pmStartPos.y += 2;
	particleManager = new ParticleManager(device, pmStartPos);
	GameEntity* particleToCel = MakeSign(celSignMat);
	exhibits[Particles]->PlaceObject(particleToCel, XMFLOAT3(-22.0f, 7.5f, 7.0f));
	GameEntity* particleToBloom = MakeSign(bloomSignMat);
	exhibits[Particles]->PlaceObject(particleToBloom, XMFLOAT3(22.0f, 7.5f, 7.0f));
	GameEntity* everythingSign = MakeSign(CreateMaterial(L"../../Assets/Textures/signs/everything sign.png", nullptr, nullptr, nullptr));
	everythingSign->GetTransform()->SetRotation(0, XM_PIDIV2, 0);
	exhibits[Particles]->PlaceObject(everythingSign, XMFLOAT3(7.0f, 7.5f, 22.0f));

	// final exhibit
	exhibits[Everything] = new Exhibit(70);
	exhibits[Everything]->AttachTo(exhibits[Particles], POSZ);

	Material* earthMat = CreateMaterial(
		L"../../Assets/Textures/earth_albedo.jpg",
		L"../../Assets/Textures/earth_normal.jpg",
		L"../../Assets/Textures/earth_roughness.jpg",
		nullptr
	);

	Material* moonMat = CreateMaterial(
		L"../../Assets/Textures/moon_albedo.jpg",
		L"../../Assets/Textures/moon_normal.jpg",
		L"../../Assets/Textures/moon_roughness.jpg",
		nullptr
	);

	sun = new GameEntity(sphere, CreateColorMaterial(XMFLOAT3(2.25f, 0.1f, 0.0f)));
	entityList.push_back(sun);
	sun->GetTransform()->SetPosition(0.0f, 0.0f, 0.0f);
	sun->GetTransform()->SetScale(6.0f, 6.0f, 6.0f);
	exhibits[Everything]->PlaceObject(sun, DirectX::XMFLOAT3(0, 10, 0));

	earth = new GameEntity(sphere, earthMat);
	entityList.push_back(earth);
	earth->GetTransform()->SetPosition(0.0f, 0.0f, 0.0f);
	earth->GetTransform()->SetScale(3.0f, 3.0f, 3.0f);

	moon = new GameEntity(sphere, moonMat);
	entityList.push_back(moon);
	moon->GetTransform()->SetPosition(1.0f, 0.0f, 0.0f);
	moon->GetTransform()->SetScale(0.75f, 0.75f, 0.75f);

	ditherObjects.push_back(earth);
	ditherObjects.push_back(moon);
	ditherObjects.push_back(sun);
}

void Game::CreateShadowMapResources()
{
	// Create shadow requirements ------------------------------------------
	shadowMapResolution = 10240; // 5120;
	shadowProjectionSize = 230; // 100

	// Create the "camera" matrices for the shadow map rendering
	XMMATRIX shView = XMMatrixLookAtLH(
		XMVectorSet(0, 30, 70, 0),
		XMVectorSet(0, 0, 70, 0),
		XMVectorSet(0, 0, 1, 0));
	shView = XMMatrixRotationX(XM_PI / 6) * shView;
	shView = XMMatrixRotationY(-XM_PI / 6) * shView;
	XMStoreFloat4x4(&shadowViewMatrix, shView);

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

	// Create a rasterizer state
	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000; // Multiplied by (smallest possible positive value storable in the depth buffer)
	shadowRastDesc.DepthBiasClamp = 0.0f;
	shadowRastDesc.SlopeScaledDepthBias = 1.0f;
	shadowRastDesc.FrontCounterClockwise = true;
	device->CreateRasterizerState(&shadowRastDesc, &shadowRasterizer);

	XMMATRIX shProj = XMMatrixOrthographicLH(shadowProjectionSize, shadowProjectionSize, 0.1f, 200.0f);
	XMStoreFloat4x4(&shadowProjectionMatrix, shProj);

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
	vertexShaderParticle = new SimpleVertexShader(device.Get(), context.Get(), GetFullPathTo_Wide(L"ParticlesVS.cso").c_str());
	pixelShaderParticle = new SimplePixelShader(device.Get(), context.Get(), GetFullPathTo_Wide(L"ParticlesPS.cso").c_str());
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

void Game::CreateParticleStates() {
	// A depth state for the particles
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // Turns off depth writing
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	device->CreateDepthStencilState(&dsDesc, particleDepthState.GetAddressOf());

	// Blend for particles (additive)
	D3D11_BLEND_DESC blend = {};
	blend.AlphaToCoverageEnable = false;
	blend.IndependentBlendEnable = false;
	blend.RenderTarget[0].BlendEnable = true;
	blend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA; // Still respect pixel shader output alpha
	blend.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	device->CreateBlendState(&blend, particleBlendState.GetAddressOf());

	// Debug rasterizer state for particles
	D3D11_RASTERIZER_DESC rd = {};
	rd.CullMode = D3D11_CULL_BACK;
	rd.DepthClipEnable = true;
	rd.FillMode = D3D11_FILL_WIREFRAME;
	device->CreateRasterizerState(&rd, particleDebugRasterState.GetAddressOf());	
}

// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();
	ResizePostProcessResources();
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
			brightness = 0;
			contrast = 1;
			blur = 1;
			transparency = 0.0f;
			bloomBlurSigma = 1.0f;
			bloomBlurRadius = 1.0f;
			for (GameEntity* entity : ditherObjects) {
				entity->GetMaterial()->SetTransparency(transparency);
			}
			break;
		}
	}
	if (Input::GetInstance().KeyPress('E')) {
		firstPerson = !firstPerson;
		Input::GetInstance().SwapMouseVisible();
	}
	if (firstPerson) {
		camera->Update(deltaTime);
	}
	particleManager->UpdateParticles(deltaTime);

	// move earth and moon
	const float earthRadius = 14.0f;
	const float moonRadius = 6.0f;
	earthRotation += XM_PIDIV4 * deltaTime;
	moonRotation += XM_PIDIV2 * deltaTime;

	XMFLOAT3 sunPos = sun->GetTransform()->GetPosition();
	earth->GetTransform()->SetPosition(sunPos.x + earthRadius * cos(earthRotation), sunPos.y, sunPos.z + earthRadius * sin(earthRotation));
	XMFLOAT3 earthPos = earth->GetTransform()->GetPosition();
	moon->GetTransform()->SetPosition(earthPos.x + moonRadius * cos(moonRotation), earthPos.y, earthPos.z + moonRadius * sin(moonRotation));
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	PreRender();
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
	
	sky->Draw(context, camera);
	DrawParticles();
	PostRender();
	
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
		ImGui::DragFloat(": bloom blur sigma", &bloomBlurSigma, 0.01f, 0.0f, 10.0f);
		ImGui::DragFloat(": bloom blur radius", &bloomBlurRadius, 0.01f, 0.0f, 5.0f);
	}
	if (exhibitIndex == Particles || exhibitIndex == Everything) {
		ImGui::DragFloat(": particles per second", &particleManager->particlesPerSecond, 1, 1, 100);
		ImGui::DragFloat(": velocity", &particleManager->velocityRange, 0.01f, 0.0f, 5.0f);
		ImGui::DragFloat(": particle size", &particleManager->particleSize, 0.01f, 0.1f, 1.0f);
		ImGui::ColorEdit4(": particle color", &particleManager->particleColor.x);
	}

	ImGui::End();
	//Assemble Together Draw Data
	ImGui::Render();
	//Render Draw Data
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	//unbind shadow map
	ID3D11ShaderResourceView* nullSRVs[16] = {};
	context->PSSetShaderResources(0, 16, nullSRVs);
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
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
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

	// After rendering the shadow maps, go back to the screen
	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthStencilView.Get());
	viewport.Width = (float)this->width;
	viewport.Height = (float)this->height;
	context->RSSetViewports(1, &viewport);
	context->RSSetState(0);
}

void Game::DrawParticles() {
	// Particle states
	context->OMSetBlendState(particleBlendState.Get(), 0, 0xffffffff);	// Additive blending
	context->OMSetDepthStencilState(particleDepthState.Get(), 0);		// No depth WRITING
	
	particleManager->CopyParticlesToGPU(context, camera);
	particleManager->DrawParticlesInternal(context, camera, pixelShaderParticle,vertexShaderParticle);

	// Reset states
	context->OMSetBlendState(0, 0, 0xffffffff);
	context->OMSetDepthStencilState(0, 0);
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
				pixelShaderBloomE->SetFloat("pixelWidth", 1.0f / width);
				pixelShaderBloomE->SetFloat("pixelHeight", 1.0f / height);
				pixelShaderBloomE->CopyAllBufferData();
				break;
		}

		// draw the full screen triangle
		context->Draw(3, 0);
	}

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
