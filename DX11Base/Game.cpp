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
	

	int i;

	for (i = 0; i<3; i++)
	{
		//renderTargetTextureArray[i] = 0;
		renderTargetViewArray[i] = 0;
		shaderResourceViewArray[i] = 0;
	}

	depthStencilBufferDR = 0;
	depthStencilViewDR = 0;

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
	delete displayVertexShader;
	delete displayPixelShader;

	delete sphereMesh;
	delete cubeMesh;

	delete materialEarth;
	delete materialCobbleStone;
	delete materialRed;
	delete materialYellow;
	delete materialSkyBox;
	delete materialSnowTracks;
	delete materialEmpty;

	delete skyBoxEntity;
	for(auto& se: sphereEntities) delete se;
	for (auto& fe : flatEntities) delete fe;
	
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
	snowTracksSRV->Release();
	snowTracksNormalSRV->Release();

	skySRV->Release();
	
	//Deferred Stuff release
	depthStencilBufferDR->Release();
	depthStencilViewDR->Release();
	rasterizerDR->Release();
	blendDR->Release();
	//depthSRV->Release();

	delete pointLightEntity;

	for (int i = 0; i < 3; i++)
	{
		shaderResourceViewArray[i]->Release();
		renderTargetViewArray[i]->Release();
		
	}

	delete deferredVertexShader;
	delete deferredPixelShader;
	delete lightingPassVertexShader;
	delete lightingPassPixelShader;
}


void Game::Init()
{
	//Initialize helper methods
	DeferredSetupInitialize();
	CameraInitialize();
	ShadersInitialize();
	ModelsInitialize();
	LoadTextures();
	MaterialsInitialize();
	SkyBoxInitialize();
	GameEntityInitialize();

	switcher = 1;
	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Game::DeferredSetupInitialize()
{
	int i;
	// Texture Discriptions
	D3D11_TEXTURE2D_DESC textureDescPosNorm;
	D3D11_TEXTURE2D_DESC textureDescDiffuse;

	// Texture for Position and Normal are 32 bit 
	textureDescPosNorm.Width = width;
	textureDescPosNorm.Height = height;
	textureDescPosNorm.MipLevels = 1;
	textureDescPosNorm.ArraySize = 1;
	textureDescPosNorm.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDescPosNorm.SampleDesc.Count = 1;
	textureDescPosNorm.SampleDesc.Quality = 0;
	textureDescPosNorm.Usage = D3D11_USAGE_DEFAULT;
	textureDescPosNorm.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDescPosNorm.CPUAccessFlags = 0;
	textureDescPosNorm.MiscFlags = 0;

	//Texture for diffuse is 8 bit 
	textureDescDiffuse.Width = width;
	textureDescDiffuse.Height = height;
	textureDescDiffuse.MipLevels = 1;
	textureDescDiffuse.ArraySize = 1;
	textureDescDiffuse.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDescDiffuse.SampleDesc.Count = 1;
	textureDescDiffuse.SampleDesc.Quality = 0;
	textureDescDiffuse.Usage = D3D11_USAGE_DEFAULT;
	textureDescDiffuse.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDescDiffuse.CPUAccessFlags = 0;
	textureDescDiffuse.MiscFlags = 0;

	//Create render target array
	ID3D11Texture2D* renderTargetTextureArray[3];

	device->CreateTexture2D(&textureDescPosNorm, 0, &renderTargetTextureArray[0]);
	device->CreateTexture2D(&textureDescPosNorm, 0, &renderTargetTextureArray[1]);
	device->CreateTexture2D(&textureDescPosNorm, 0, &renderTargetTextureArray[2]);

	// Render target desc for 32bit
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc32;
	ZeroMemory(&renderTargetViewDesc32, sizeof(renderTargetViewDesc32));
	renderTargetViewDesc32.Format = textureDescPosNorm.Format;
	renderTargetViewDesc32.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc32.Texture2D.MipSlice = 0;

	// Render target desc for 8bit
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc8;
	ZeroMemory(&renderTargetViewDesc8, sizeof(renderTargetViewDesc8));
	renderTargetViewDesc8.Format = textureDescPosNorm.Format;
	renderTargetViewDesc8.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc8.Texture2D.MipSlice = 0;

	//Create the render target arrays
	device->CreateRenderTargetView(renderTargetTextureArray[0], &renderTargetViewDesc32, &renderTargetViewArray[0]);
	device->CreateRenderTargetView(renderTargetTextureArray[1], &renderTargetViewDesc32, &renderTargetViewArray[1]);
	device->CreateRenderTargetView(renderTargetTextureArray[2], &renderTargetViewDesc8, &renderTargetViewArray[2]);

	//Creste SRV desc
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDescDR;
	ZeroMemory(&shaderResourceViewDescDR, sizeof(shaderResourceViewDescDR));
	shaderResourceViewDescDR.Format = textureDescPosNorm.Format;
	shaderResourceViewDescDR.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDescDR.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDescDR.Texture2D.MipLevels = 1;

	//Create SRV's
	for (i = 0; i < 3; i++)
	{
		device->CreateShaderResourceView(renderTargetTextureArray[i], &shaderResourceViewDescDR, &shaderResourceViewArray[i]);
		
	}

	//Release render target texture array
	for (i = 0; i < 3; i++)
	{
		renderTargetTextureArray[i]->Release();
	}

	//Create depth buffer desc
	D3D11_TEXTURE2D_DESC depthBufferDescDR;
	ZeroMemory(&depthBufferDescDR, sizeof(depthBufferDescDR));
	depthBufferDescDR.Width = width;
	depthBufferDescDR.Height = height;
	depthBufferDescDR.MipLevels = 1;
	depthBufferDescDR.ArraySize = 1;
	//depthBufferDescDR.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDescDR.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthBufferDescDR.SampleDesc.Count = 1;
	depthBufferDescDR.SampleDesc.Quality = 0;
	depthBufferDescDR.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDescDR.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthBufferDescDR.CPUAccessFlags = 0;
	depthBufferDescDR.MiscFlags = 0;

	device->CreateTexture2D(&depthBufferDescDR, 0, &depthStencilBufferDR);
	
	//Create depth stencil desc
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDescDR;
	ZeroMemory(&depthStencilViewDescDR, sizeof(depthStencilViewDescDR));
	depthStencilViewDescDR.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDescDR.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDescDR.Texture2D.MipSlice = 0;
	
	//Create depth stencil view
	device->CreateDepthStencilView(depthStencilBufferDR, &depthStencilViewDescDR, &depthStencilViewDR);


	/*D3D11_SHADER_RESOURCE_VIEW_DESC depthDescSRV;
	ZeroMemory(&depthDescSRV, sizeof(depthDescSRV));
	depthDescSRV.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	depthDescSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	depthDescSRV.Texture2D.MostDetailedMip = 0;
	depthDescSRV.Texture2D.MipLevels = 1;

	depthSRV = nullptr;
	device->CreateShaderResourceView(depthStencilBufferDR, &depthDescSRV, &depthSRV);*/

	//Setup the viewport for rendering.
	viewportDR.Width = width;
	viewportDR.Height = height;
	viewportDR.MinDepth = 0.0f;
	viewportDR.MaxDepth = 1.0f;
	viewportDR.TopLeftX = 0.0f;
	viewportDR.TopLeftY = 0.0f;

	//context->OMSetRenderTargets(3, renderTargetViewArray, depthStencilViewDR);

	//Setup rasterizer state 
	D3D11_RASTERIZER_DESC rasterizerDescDR;
	ZeroMemory(&rasterizerDescDR, sizeof(rasterizerDescDR));
	rasterizerDescDR.CullMode = D3D11_CULL_NONE;
	rasterizerDescDR.FillMode = D3D11_FILL_SOLID;
	rasterizerDescDR.DepthClipEnable = false;
	
	device->CreateRasterizerState(&rasterizerDescDR, &rasterizerDR);

	//Setup blend state 
	D3D11_BLEND_DESC blendDescDR;
	ZeroMemory(&blendDescDR, sizeof(blendDescDR));
	blendDescDR.AlphaToCoverageEnable = false;
	blendDescDR.IndependentBlendEnable = false;
	blendDescDR.RenderTarget[0].BlendEnable = true;
	blendDescDR.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDescDR.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	blendDescDR.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDescDR.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDescDR.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendDescDR.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDescDR.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	device->CreateBlendState(&blendDescDR, &blendDR);


}

void Game::CameraInitialize()
{
	camera = new Camera(0, 1, -6);
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

	deferredVertexShader = new SimpleVertexShader(device, context);
	if (!deferredVertexShader->LoadShaderFile(L"Debug/DeferredVertexShader.cso"))
		deferredVertexShader->LoadShaderFile(L"DeferredVertexShader.cso");

	deferredPixelShader = new SimplePixelShader(device, context);
	if (!deferredPixelShader->LoadShaderFile(L"Debug/DeferredPixelShader.cso"))
		deferredPixelShader->LoadShaderFile(L"DeferredPixelShader.cso");

	displayVertexShader = new SimpleVertexShader(device, context);
	if (!displayVertexShader->LoadShaderFile(L"Debug/DisplayVertexShader.cso"))
		displayVertexShader->LoadShaderFile(L"DisplayVertexShader.cso");

	displayPixelShader = new SimplePixelShader(device, context);
	if (!displayPixelShader->LoadShaderFile(L"Debug/DisplayPixelShader.cso"))
		displayPixelShader->LoadShaderFile(L"DisplayPixelShader.cso");

	lightingPassVertexShader = new SimpleVertexShader(device, context);
	if (!lightingPassVertexShader->LoadShaderFile(L"Debug/LightingPassVertexShader.cso"))
		lightingPassVertexShader->LoadShaderFile(L"LightingPassVertexShader.cso");

	lightingPassPixelShader = new SimplePixelShader(device, context);
	if (!lightingPassPixelShader->LoadShaderFile(L"Debug/LightingPassPixelShader.cso"))
		lightingPassPixelShader->LoadShaderFile(L"LightingPassPixelShader.cso");

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
	CreateWICTextureFromFile(device, context, L"Textures/snowTracks.tif", 0, &snowTracksSRV);
	CreateWICTextureFromFile(device, context, L"Textures/snowTracksNormal.tif", 0, &snowTracksNormalSRV);
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
	materialSnowTracks = new Material(basePixelShader, baseVertexShader, snowTracksSRV, snowTracksNormalSRV, sampler);
	materialEmpty = new Material(basePixelShader, baseVertexShader, 0, plainNormalMapSRV, sampler);

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

	pointLightEntity = new GameEntity(sphereMesh, NULL);
	pointLightEntity->SetPosition(1.0f, 0.0f, 0.0f);
	pointLightEntity->SetScale(2.0f, 2.0f, 2.0f);
	
	GameEntity* sphere0 = new GameEntity(sphereMesh, materialCobbleStone);
	GameEntity* sphere1 = new GameEntity(sphereMesh, materialCobbleStone);
	GameEntity* sphere2 = new GameEntity(sphereMesh, materialCobbleStone);
	GameEntity* sphere3 = new GameEntity(sphereMesh, materialCobbleStone);
	GameEntity* sphere4 = new GameEntity(sphereMesh, materialCobbleStone);
	GameEntity* sphere5 = new GameEntity(sphereMesh, materialCobbleStone);
	GameEntity* sphere6 = new GameEntity(sphereMesh, materialCobbleStone);
	GameEntity* sphere7 = new GameEntity(sphereMesh, materialCobbleStone);
	GameEntity* sphere8 = new GameEntity(sphereMesh, materialCobbleStone);

	sphereEntities.push_back(sphere0);
	sphereEntities.push_back(sphere1);
	sphereEntities.push_back(sphere2);
	sphereEntities.push_back(sphere3);
	sphereEntities.push_back(sphere4);
	sphereEntities.push_back(sphere5);
	sphereEntities.push_back(sphere6);
	sphereEntities.push_back(sphere7);
	sphereEntities.push_back(sphere8);

	sphereEntities[0]->SetPosition(0, 0, 2);
	sphereEntities[1]->SetPosition(2, 0, 2);
	sphereEntities[2]->SetPosition(-2, 0, 2);
	sphereEntities[3]->SetPosition(0, 0, 0);
	sphereEntities[4]->SetPosition(2, 0, 0);
	sphereEntities[5]->SetPosition(-2, 0, 0);
	sphereEntities[6]->SetPosition(0, 0, -2);
	sphereEntities[7]->SetPosition(2, 0, -2);
	sphereEntities[8]->SetPosition(-2, 0, -2);


	GameEntity* flat0 = new GameEntity(cubeMesh, materialRed);
	GameEntity* flat1 = new GameEntity(cubeMesh, materialRed);
	GameEntity* flat2 = new GameEntity(cubeMesh, materialRed);
	GameEntity* flat3 = new GameEntity(cubeMesh, materialRed);
	
	flatEntities.push_back(flat0);
	flatEntities.push_back(flat1);
	flatEntities.push_back(flat2);
	flatEntities.push_back(flat3);

	flatEntities[0]->SetScale(5.0f, 0.01f, 5.0f);
	flatEntities[1]->SetScale(5.0f, 0.01f, 5.0f);
	flatEntities[2]->SetScale(5.0f, 0.01f, 5.0f);
	flatEntities[3]->SetScale(5.0f, 0.01f, 5.0f);

	flatEntities[0]->SetPosition(0, -1.5f, 0);
	flatEntities[1]->SetPosition(4.5f, 0, 0);
	flatEntities[2]->SetPosition(0, 0, 4.5f);
	flatEntities[3]->SetPosition(-4.5f, 0, 0);

	flatEntities[1]->SetRotation(0, 0, -1.6f);
	flatEntities[2]->SetRotation(1.6f, 0, 0);
	flatEntities[3]->SetRotation(0, 0, 1.6f);
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
	
	//Update Spheres
	for (int i = 0; i <= 8; i++)
	{
		sphereEntities[i]->UpdateWorldMatrix();
	}

	//Update Flats
	for (int i = 0; i <= 3; i++)
	{
		flatEntities[i]->UpdateWorldMatrix();
	}

	pointLightEntity->UpdateWorldMatrix();

	//Switch g-buffer
	if (GetAsyncKeyState('1') & 0x8000) switcher = 1;
	if (GetAsyncKeyState('2') & 0x8000) switcher = 2;
	if (GetAsyncKeyState('3') & 0x8000) switcher = 3;
	//if (GetAsyncKeyState('4') & 0x8000) switcher = 4;


	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();
}

void Game::Draw(float deltaTime, float totalTime)
{
	// Background color (Cornflower Blue in this case) for clearing
	const float color[4] = { 0.6f, 0.6f, 0.6f, 0.0f };
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	//Render Spheres
	//for (int i = 0; i <= 8 ; i++) 
	//{
	//	render.RenderProcess(sphereEntities[i], vertexBuffer, indexBuffer, baseVertexShader, basePixelShader, camera, context);
	//}

	//Render Flats
	/*for (int i = 0; i <= 3; i++)
	{
		render.RenderProcess(flatEntities[i], vertexBuffer, indexBuffer, baseVertexShader, basePixelShader, camera, context);
	}

	render.RenderSkyBox(cubeMesh, vertexBuffer, indexBuffer, skyVertexShader, skyPixelShader, camera, context, skyRasterizerState, skyDepthState, skySRV);*/

//-----------------------------
	context->OMSetRenderTargets(3, renderTargetViewArray, depthStencilViewDR);
	context->RSSetViewports(1, &viewportDR);

	context->ClearRenderTargetView(renderTargetViewArray[0], color);
	context->ClearRenderTargetView(renderTargetViewArray[1], color);
	context->ClearRenderTargetView(renderTargetViewArray[2], color);

	context->ClearDepthStencilView(depthStencilViewDR, D3D11_CLEAR_DEPTH , 1.0f, 0);

	for (int i = 0; i <= 3; i++)
	{
		vertexBuffer = flatEntities[i]->GetMesh()->GetVertexBuffer();
		indexBuffer = flatEntities[i]->GetMesh()->GetIndexBuffer();

		deferredVertexShader->SetMatrix4x4("world", *flatEntities[i]->GetWorldMatrix());
		deferredVertexShader->SetMatrix4x4("view", camera->GetView());
		deferredVertexShader->SetMatrix4x4("projection", camera->GetProjection());

		deferredVertexShader->CopyAllBufferData();
		deferredVertexShader->SetShader();

		deferredPixelShader->SetShaderResourceView("textureSRV", flatEntities[i]->GetMaterial()->GetMaterialSRV());
		deferredPixelShader->SetShaderResourceView("normalMapSRV", flatEntities[i]->GetMaterial()->GetNormalSRV());
		deferredPixelShader->SetSamplerState("basicSampler", flatEntities[i]->GetMaterial()->GetMaterialSampler());

		deferredPixelShader->CopyAllBufferData();
		deferredPixelShader->SetShader();

		context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

		context->DrawIndexed(flatEntities[i]->GetMesh()->GetIndexCount(), 0, 0);
	}

	for (int i = 0; i <= 8; i++)
	{
		vertexBuffer = sphereEntities[i]->GetMesh()->GetVertexBuffer();
		indexBuffer = sphereEntities[i]->GetMesh()->GetIndexBuffer();

		deferredVertexShader->SetMatrix4x4("world", *sphereEntities[i]->GetWorldMatrix());
		deferredVertexShader->SetMatrix4x4("view", camera->GetView());
		deferredVertexShader->SetMatrix4x4("projection", camera->GetProjection());

		deferredVertexShader->CopyAllBufferData();
		deferredVertexShader->SetShader();

		deferredPixelShader->SetShaderResourceView("textureSRV", sphereEntities[i]->GetMaterial()->GetMaterialSRV());
		deferredPixelShader->SetShaderResourceView("normalMapSRV", sphereEntities[i]->GetMaterial()->GetNormalSRV());
		deferredPixelShader->SetSamplerState("basicSampler", sphereEntities[i]->GetMaterial()->GetMaterialSampler());

		deferredPixelShader->CopyAllBufferData();
		deferredPixelShader->SetShader();

		context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

		context->DrawIndexed(sphereEntities[i]->GetMesh()->GetIndexCount(), 0, 0);
	}

	/*deferredPixelShader->SetShaderResourceView("textureSRV", 0);
	deferredPixelShader->SetShaderResourceView("normalMapSRV", 0);
	deferredPixelShader->SetSamplerState("basicSampler", 0);*/
//-----------------------------
/*
	context->OMSetRenderTargets(1, &backBufferRTV, 0);
	context->RSSetViewports(1, &viewport);

	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);	

	displayVertexShader->SetShader();

	switch (switcher)
	{
	case 1:
		displayPixelShader->SetShaderResourceView("Texture", shaderResourceViewArray[0]);
		break;
	case 2:
		displayPixelShader->SetShaderResourceView("Texture", shaderResourceViewArray[1]);
		break;
	case 3:
		displayPixelShader->SetShaderResourceView("Texture", shaderResourceViewArray[2]);
		break;
	//case 4:
		//displayPixelShader->SetShaderResourceView("Texture", depthSRV);
		//break;
	default:
		break;
	}
	displayPixelShader->SetSamplerState("Sampler", sampler);
	displayPixelShader->CopyAllBufferData();
	displayPixelShader->SetShader();

	ID3D11Buffer* nothing = 0;
	context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
	context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	// Actually draw exactly 3 vertices
	context->Draw(3, 0);
	displayPixelShader->SetShaderResourceView("Texture", 0);
	/**/
//-------------------	

	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);
	context->RSSetViewports(1, &viewport);

	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	context->RSSetState(rasterizerDR);
	float blend[4] = { 1,1,1,1 };
	context->OMSetBlendState(blendDR, blend, 0xFFFFFFFF);

	vertexBuffer = pointLightEntity->GetMesh()->GetVertexBuffer();
	indexBuffer = pointLightEntity->GetMesh()->GetIndexBuffer();

	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	lightingPassVertexShader->SetMatrix4x4("world", *pointLightEntity->GetWorldMatrix());
	lightingPassVertexShader->SetMatrix4x4("view", camera->GetView());
	lightingPassVertexShader->SetMatrix4x4("projection", camera->GetProjection());

	lightingPassVertexShader->CopyAllBufferData();
	lightingPassVertexShader->SetShader();

	lightingPassPixelShader->SetShaderResourceView("positionGB", shaderResourceViewArray[0]);
	lightingPassPixelShader->SetShaderResourceView("normalGB", shaderResourceViewArray[1]);
	lightingPassPixelShader->SetShaderResourceView("diffuseGB", shaderResourceViewArray[2]);

	lightingPassPixelShader->SetSamplerState("basicSampler", sampler);

	lightingPassPixelShader->SetFloat3("cameraPosition", camera->GetPosition());

	lightingPassPixelShader->SetFloat3("lightColor", XMFLOAT3(1.0f, 1.0f, 1.0f));
	lightingPassPixelShader->SetFloat3("lightPos", pointLightEntity->GetPosition());

	lightingPassPixelShader->CopyAllBufferData();
	lightingPassPixelShader->SetShader();

	context->DrawIndexed(pointLightEntity->GetMesh()->GetIndexCount(), 0, 0);

	context->RSSetState(NULL);
	context->OMSetBlendState(NULL, blend, 0xFFFFFFFF);

	//--------------------------	
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
