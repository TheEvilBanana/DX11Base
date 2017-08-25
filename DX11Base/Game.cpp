#include "Game.h"

#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"

#define max(a,b) (((a) > (b)) ? (a):(b))
#define min(a,b) (((a) < (b)) ? (a):(b))

Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,		   // The application's handle
		"DirectX Game",	   // Text for the window's title bar
		1280,			   // Width of the window's client area
		720,			   // Height of the window's client area
		true)			   // Show extra stats (fps) in title bar?
{
	// Initialize fields
	vertexBuffer = 0;
	indexBuffer = 0;
	baseVertexShader = 0;
	basePixelShader = 0;
	camera = 0;
	

#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.");
#endif
}


Game::~Game()
{
	delete camera;

	delete basePixelShader;
	delete baseVertexShader;
	delete skyVertexShader;
	delete skyPixelShader;

	delete sphereMesh;
	delete cubeMesh;

	delete materialEarth;
	delete materialCobbleStone;
	delete materialRed;
	delete materialYellow;
	delete materialSkyBox;

	delete skyBoxEntity;
	delete globeEntity;
	delete flatEntity;
	
	skyDepthState->Release();
	skyRasterizerState->Release();

	sampler->Release();
	
	earthDayMapSRV->Release();
	earthNormalMapSRV->Release();
	cobbleStoneSRV->Release();
	cobbleStoneNormalSRV->Release();
	plainRedSRV->Release();
	plainYellowSRV->Release();
	plainNormalMapSRV->Release();
	skySRV->Release();
	


}


void Game::Init()
{
	//Initialize helper methods
	CameraInitialize();
	ShadersInitialize();
	ModelsInitialize();
	LoadTextures();
	MaterialsInitialize();
	SkyBoxInitialize();
	GameEntityInitialize();

	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Game::CameraInitialize()
{
	camera = new Camera(0, 0, -4);
	camera->UpdateProjectionMatrix((float)width / height);
}

void Game::ShadersInitialize()
{
	baseVertexShader = new SimpleVertexShader(device, context);
	if (!baseVertexShader->LoadShaderFile(L"Debug/BaseVertexShader.cso"))
		baseVertexShader->LoadShaderFile(L"BaseVertexShader.cso");

	basePixelShader = new SimplePixelShader(device, context);
	if (!basePixelShader->LoadShaderFile(L"Debug/BasePixelShader.cso"))
		basePixelShader->LoadShaderFile(L"BasePixelShader.cso");

	skyVertexShader = new SimpleVertexShader(device, context);
	if (!skyVertexShader->LoadShaderFile(L"Debug/SkyBoxVertexShader.cso"))
		skyVertexShader->LoadShaderFile(L"SkyBoxVertexShader.cso");

	skyPixelShader = new SimplePixelShader(device, context);
	if (!skyPixelShader->LoadShaderFile(L"Debug/SkyBoxPixelShader.cso"))
		skyPixelShader->LoadShaderFile(L"SkyBoxPixelShader.cso");
}

void Game::ModelsInitialize()
{
	sphereMesh = new Mesh("Models/sphere.obj", device);
	cubeMesh = new Mesh("Models/cube.obj", device);
}

void Game::LoadTextures()
{
	CreateWICTextureFromFile(device, context, L"Textures/earth_daymap.jpg", 0, &earthDayMapSRV);
	CreateWICTextureFromFile(device, context, L"Textures/earth_normal_map.tif", 0, &earthNormalMapSRV);
	CreateWICTextureFromFile(device, context, L"Textures/rock.jpg", 0, &cobbleStoneSRV);
	CreateWICTextureFromFile(device, context, L"Textures/rockNormals.jpg", 0, &cobbleStoneNormalSRV);
	CreateWICTextureFromFile(device, context, L"Textures/red.jpg", 0, &plainRedSRV);
	CreateWICTextureFromFile(device, context, L"Textures/yellow.jpg", 0, &plainYellowSRV);
	CreateWICTextureFromFile(device, context, L"Textures/plainNormal.png", 0, &plainNormalMapSRV);
}

void Game::MaterialsInitialize()
{
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, &sampler);


	materialEarth = new Material(basePixelShader, baseVertexShader, earthDayMapSRV, earthNormalMapSRV, sampler);
	materialCobbleStone = new Material(basePixelShader, baseVertexShader, cobbleStoneSRV, cobbleStoneNormalSRV, sampler);
	materialRed = new Material(basePixelShader, baseVertexShader, plainRedSRV, plainNormalMapSRV, sampler);
	materialYellow = new Material(basePixelShader, baseVertexShader, plainYellowSRV, plainNormalMapSRV, sampler);
	materialSkyBox = new Material(skyPixelShader, skyVertexShader, skySRV, plainNormalMapSRV, sampler);
}

void Game::SkyBoxInitialize()
{
	CreateDDSTextureFromFile(device, L"Textures/SunnyCubeMap.dds", 0, &skySRV);

	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_FRONT;
	rasterizerDesc.DepthClipEnable = true;
	device->CreateRasterizerState(&rasterizerDesc, &skyRasterizerState);

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&depthStencilDesc, &skyDepthState);
}

void Game::GameEntityInitialize()
{
	skyBoxEntity = new GameEntity(cubeMesh, materialSkyBox);

	globeEntity = new GameEntity(sphereMesh, materialEarth);
	globeEntity->SetPosition(0, 1, 0);
	//globeEntity->SetScale(2, 2, 2);

	flatEntity = new GameEntity(cubeMesh, materialYellow);
	flatEntity->SetPosition(0, -1, 0);
	flatEntity->SetScale(5.0f, 0.01f, 5.0f);
	
}

void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	// Update the projection matrix assuming the
	// camera exists
	if (camera)
		camera->UpdateProjectionMatrix((float)width / height);
}

void Game::Update(float deltaTime, float totalTime)
{
	camera->Update(deltaTime);

	globeEntity->Rotate(0, -deltaTime * 0.2f, 0);

	globeEntity->UpdateWorldMatrix();
	flatEntity->UpdateWorldMatrix();

	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();
}

void Game::Draw(float deltaTime, float totalTime)
{
	// Background color (Cornflower Blue in this case) for clearing
	const float color[4] = { 0.6f, 0.6f, 0.6f, 0.0f };

	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


	//render.RenderShadowMap(shadowVertexShader, shadowDepthStencil, shadowRasterizer, context, globeEntity, flatEntity, vertexBuffer, indexBuffer, this->backBufferRTV, this->depthStencilView, shadowMapSize, this->width, this->height, shadowViewMatrix, shadowProjectionMatrix);
	

	render.RenderProcess(globeEntity, vertexBuffer, indexBuffer, baseVertexShader, basePixelShader, camera, context);
	render.RenderProcess(flatEntity, vertexBuffer, indexBuffer, baseVertexShader, basePixelShader, camera, context);

	render.RenderSkyBox(cubeMesh, vertexBuffer, indexBuffer, skyVertexShader, skyPixelShader, camera, context, skyRasterizerState, skyDepthState, skySRV);
	
	swapChain->Present(0, 0);
}

#pragma region Mouse Input

// --------------------------------------------------------
// Helper method for mouse clicking.  We get this information
// from the OS-level messages anyway, so these helpers have
// been created to provide basic mouse input if you want it.
// --------------------------------------------------------
void Game::OnMouseDown(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;

	// Caputure the mouse so we keep getting mouse move
	// events even if the mouse leaves the window.  we'll be
	// releasing the capture once a mouse button is released
	SetCapture(hWnd);
}

// --------------------------------------------------------
// Helper method for mouse release
// --------------------------------------------------------
void Game::OnMouseUp(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// We don't care about the tracking the cursor outside
	// the window anymore (we're not dragging if the mouse is up)
	ReleaseCapture();
}

// --------------------------------------------------------
// Helper method for mouse movement.  We only get this message
// if the mouse is currently over the window, or if we're 
// currently capturing the mouse.
// --------------------------------------------------------
void Game::OnMouseMove(WPARAM buttonState, int x, int y)
{
	// Check left mouse button
	if (buttonState & 0x0001) {
		float xDiff = (x - prevMousePos.x) * 0.005f;
		float yDiff = (y - prevMousePos.y) * 0.005f;
		camera->Rotate(yDiff, xDiff);
	}

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;
}

// --------------------------------------------------------
// Helper method for mouse wheel scrolling.  
// WheelDelta may be positive or negative, depending 
// on the direction of the scroll
// --------------------------------------------------------
void Game::OnMouseWheel(float wheelDelta, int x, int y)
{
	// Add any custom code here...
}

#pragma endregion
