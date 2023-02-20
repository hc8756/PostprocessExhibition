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
		true)			   // Show extra stats (fps) in title bar?
{
	camera = 0;
	ambientColor= XMFLOAT3(0.2f, 0.2f, 0.2f);
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

	delete vertexShaderFull;
	vertexShaderFull = nullptr;

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
	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	device->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf());
	//create srv's
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> earthAlbedoSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> earthMetalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> earthRoughnessSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> earthNormalSRV;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> moonAlbedoSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> moonRoughnessSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> moonNormalSRV;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneAlbedoSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneMetalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneRoughnessSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneNormalSRV;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skySRV;
	//Load texture

	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/earth_albedo.jpg").c_str(), 0, earthAlbedoSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/universal_metal.jpg").c_str(), 0, earthMetalSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/earth_roughness.jpg").c_str(), 0, earthRoughnessSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/earth_normal.jpg").c_str(), 0, earthNormalSRV.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/moon_albedo.jpg").c_str(), 0, moonAlbedoSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/moon_roughness.jpg").c_str(), 0, moonRoughnessSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/moon_normal.jpg").c_str(), 0, moonNormalSRV.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/cobblestone/cobblestone_albedo.png").c_str(), 0, cobblestoneAlbedoSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/cobblestone/cobblestone_metal.png").c_str(), 0, cobblestoneMetalSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/cobblestone/cobblestone_roughness.png").c_str(), 0, cobblestoneRoughnessSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/cobblestone/cobblestone_normal.png").c_str(), 0, cobblestoneNormalSRV.GetAddressOf());

	//CreateDDSTextureFromFile(device.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/SunnyCubeMap.dds").c_str(), 0, skySRV.GetAddressOf());
	
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
	Material* material1 = new Material(DirectX::XMFLOAT3(+2.5f, +2.5f, +2.5f),0.0f,pixelShader,vertexShader);
	material1->AddSamplerState("BasicSamplerState", samplerState);
	material1->AddTextureSRV("Albedo", earthAlbedoSRV);
	material1->AddTextureSRV("MetalnessMap", earthMetalSRV);
	material1->AddTextureSRV("RoughnessMap", earthRoughnessSRV);
	material1->AddTextureSRV("NormalMap", earthNormalSRV);

	Material* material2 = new Material(DirectX::XMFLOAT3(+2.5f, +2.5f, +2.5f), 0.0f, pixelShader, vertexShader);
	material2->AddSamplerState("BasicSamplerState", samplerState);
	material2->AddTextureSRV("Albedo", moonAlbedoSRV);
	material2->AddTextureSRV("MetalnessMap", earthMetalSRV);
	material2->AddTextureSRV("RoughnessMap", moonRoughnessSRV);
	material2->AddTextureSRV("NormalMap", moonNormalSRV);

	Material* material3 = new Material(DirectX::XMFLOAT3(+2.5f, +2.5f, +2.5f), 0.0f, pixelShader, vertexShader);
	material3->AddSamplerState("BasicSamplerState", samplerState);
	material3->AddTextureSRV("Albedo", cobblestoneAlbedoSRV);
	material3->AddTextureSRV("MetalnessMap", cobblestoneMetalSRV);
	material3->AddTextureSRV("RoughnessMap", cobblestoneRoughnessSRV);
	material3->AddTextureSRV("NormalMap", cobblestoneNormalSRV);

	materialList.push_back(material1);
	materialList.push_back(material2);
	materialList.push_back(material3);

	//Exhibit::cube = cube;
	//Exhibit::surface = material3;

	//Create entities
	GameEntity* entity1 = new GameEntity(sphere,material1);
	GameEntity* entity2 = new GameEntity(sphere, material2);

	//add entities to the entitiy list
	entityList.push_back(entity1);
	entityList.push_back(entity2);

	//move model entities around so that they don't overlap
	entityList[0]->GetTransform()->SetPosition(0.0f, 0.0f, 0.0f);
	entityList[1]->GetTransform()->SetPosition(1.0f, 0.0f, 0.0f);


	//create camera 
	camera = new Camera(0, 0, -10, 5.0f, 0.2f, XM_PIDIV4, (float)width / height);

	//default normal and uv values
	defNormal = XMFLOAT3(+0.0f, +0.0f, -1.0f);
	defUV = XMFLOAT2(+0.0f, +0.0f);

	//Sunlight from the left
	Light directionalLight1 = {};
	directionalLight1.Type = 0;
	directionalLight1.Direction= XMFLOAT3(1.0f, 0.0f, 0.0f);
	directionalLight1.Color= XMFLOAT3(0.1f, 0.1f, 0.1f);
	directionalLight1.Intensity = 10.0f;
	//add entities to the entitiy list
	lightList.push_back(directionalLight1);

	firstPerson = true;
	Input::GetInstance().SwapMouseVisible();
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
	vertexShaderSky = new SimpleVertexShader(device.Get(), context.Get(), GetFullPathTo_Wide(L"VertexShaderSky.cso").c_str());
	pixelShaderSky = new SimplePixelShader(device.Get(), context.Get(), GetFullPathTo_Wide(L"PixelShaderSky.cso").c_str());
	vertexShaderFull = new SimpleVertexShader(device.Get(), context.Get(), GetFullPathTo_Wide(L"VertexShaderFull.cso").c_str());
	pixelShaderSobel = new SimplePixelShader(device.Get(), context.Get(), GetFullPathTo_Wide(L"PixelShaderSobel.cso").c_str());
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
	ppSRV.Reset();
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
	device->CreateRenderTargetView(sceneNormalsTexture.Get(), 0, sceneNormalsRTV.GetAddressOf());
	device->CreateRenderTargetView(sceneDepthsTexture.Get(), 0, sceneDepthRTV.GetAddressOf());

	// Create the Shader Resource Views (null descriptions use default settings)
	device->CreateShaderResourceView(ppTexture.Get(), 0, ppSRV.GetAddressOf());
	device->CreateShaderResourceView(sceneNormalsTexture.Get(), 0, sceneNormalsSRV.GetAddressOf());
	device->CreateShaderResourceView(sceneDepthsTexture.Get(), 0, sceneDepthSRV.GetAddressOf());
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Example input checking: Quit if the escape key is pressed
	if (Input::GetInstance().KeyDown(VK_ESCAPE))
		Quit();

	//make items rotate along y axis
	static float increment = 0.5f;
	increment += 0.0005f;
	
	//scale correctly 
	entityList[0]->GetTransform()->SetScale(1.0f, 1.0f, 1.0f); 
	entityList[1]->GetTransform()->SetScale(0.25f, 0.25f, 0.25f);
	entityList[0]->GetTransform()->SetRotation(0.0f, increment/2, 0.0f);
	entityList[1]->GetTransform()->SetPosition(2*sin(increment), 0.0f, 2*cos(increment));

	//code that will alter fov based on user input
	float fov = camera->GetFoV();
	if (Input::GetInstance().KeyDown('P')) {
		fov += 1 * deltaTime;
		camera->SetFoV(fov);
	}
	if (Input::GetInstance().KeyDown('O')) {
		fov -= 1 * deltaTime;
		camera->SetFoV(fov);
	}

	if (Input::GetInstance().KeyPress('R')) {
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

	//call game entity drawing method for each entitiy
	//also set up lighting stuff
	for (int i = 0; i < entityList.size(); i++) {
		entityList[i]->GetMaterial()->GetPixelShader()->SetFloat3("ambientColor",ambientColor);
		entityList[i]->GetMaterial()->GetPixelShader()->SetData("lights", &lightList[0], sizeof(Light) * (int)lightList.size());
		entityList[i]->Draw(context,camera);
	}
	//draw sky
	sky->Draw(context, camera);
	PostRender();

	//draw ImGui elements
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	//Create ImGui Test Window
	ImGui::Begin("Test");
	ImGui::End();
	//Assemble Together Draw Data
	ImGui::Render();
	//Render Draw Data
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

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

	// Clear all render targets
	context->ClearRenderTargetView(ppRTV.Get(), color);
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

void Game::PostRender()
{

	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), 0);

	// Set up post process shaders
	vertexShaderFull->SetShader();
	pixelShaderSobel->SetShader();
	pixelShaderSobel->SetShaderResourceView("image", ppSRV.Get());
	pixelShaderSobel->SetSamplerState("samplerOptions", clampSampler.Get());
	pixelShaderSobel->SetFloat("pixelWidth", 1.0f / width);
	pixelShaderSobel->SetFloat("pixelHeight", 1.0f / height);
	pixelShaderSobel->CopyAllBufferData();

	// Draw 3 vertices
	context->Draw(3, 0);
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