#pragma once

#include <DirectXMath.h>

#include "DXCore.h"
#include "Camera.h"
#include "Mesh.h"
#include "Material.h"
#include "GameEntity.h"
#include "Render.h"

using namespace DirectX;

class Game : public DXCore
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

	// Overridden mouse input helper methods
	void OnMouseDown(WPARAM buttonState, int x, int y);
	void OnMouseUp(WPARAM buttonState, int x, int y);
	void OnMouseMove(WPARAM buttonState, int x, int y);
	void OnMouseWheel(float wheelDelta, int x, int y);

private:

	// Initialization helper methods 
	void DeferredSetupInitialize();
	void CameraInitialize();
	void ShadersInitialize();
	void ModelsInitialize();
	void LoadTextures();
	void MaterialsInitialize();
	void SkyBoxInitialize();
	void GameEntityInitialize();
	void LightsInitialize();


	//Deferred Rendering Requirements
	
	ID3D11RenderTargetView* renderTargetViewArray[3];
	ID3D11ShaderResourceView* shaderResourceViewArray[3];
	ID3D11Texture2D* depthStencilBufferDR;
	ID3D11DepthStencilView* depthStencilViewDR;
	D3D11_VIEWPORT viewportDR;

	ID3D11RasterizerState* rasterizerDR;
	ID3D11BlendState* blendDR;

	//ID3D11ShaderResourceView* depthSRV;

	int switcher;

	SimpleVertexShader* deferredVertexShader;
	SimplePixelShader* deferredPixelShader;
	SimpleVertexShader* lightingPassVertexShader;
	SimplePixelShader* lightingPassPixelShader;

	SimpleVertexShader* dirLightVertexShader;
	SimplePixelShader* dirLightPixelShader;

	GameEntity* pointLightEntity;
	GameEntity* pointLightEntity2;
	std::vector<GameEntity*> pointLightEntities;
	// Buffers to hold actual geometry data
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;


	//Declare shader variables
	SimpleVertexShader* baseVertexShader;
	SimplePixelShader* basePixelShader;
	SimpleVertexShader* skyVertexShader;
	SimplePixelShader* skyPixelShader;
	SimpleVertexShader* displayVertexShader;
	SimplePixelShader* displayPixelShader;

	// Sampler for wrapping textures
	ID3D11SamplerState* sampler;

	//Texture Shader Resource Views(SRVs)
	ID3D11ShaderResourceView* earthDayMapSRV;
	ID3D11ShaderResourceView* cobbleStoneSRV;
	ID3D11ShaderResourceView* snowTracksSRV;
	ID3D11ShaderResourceView* plainRedSRV;
	ID3D11ShaderResourceView* plainYellowSRV;

	//Normal Shader Resource Views(SRVs)
	ID3D11ShaderResourceView* plainNormalMapSRV;
	ID3D11ShaderResourceView* earthNormalMapSRV;
	ID3D11ShaderResourceView* cobbleStoneNormalSRV;
	ID3D11ShaderResourceView* snowTracksNormalSRV;

	//Sky Box Stuff
	ID3D11ShaderResourceView* skySRV;
	ID3D11RasterizerState* skyRasterizerState;
	ID3D11DepthStencilState* skyDepthState;

	//Mesh Class
	Mesh* sphereMesh;
	Mesh* cubeMesh;

	//Material Class
	Material* materialEarth;
	Material* materialCobbleStone;
	Material* materialRed;
	Material* materialYellow;
	Material* materialSkyBox;
	Material* materialSnowTracks;
	Material* materialEmpty;

	//Game Entity Class
	GameEntity* skyBoxEntity;
	std::vector<GameEntity*> sphereEntities;
	std::vector<GameEntity*> flatEntities;

	//Render Class
	Render render;

	//Camera class
	Camera* camera;

	// The matrices to go from model space to screen space
	XMFLOAT4X4 worldMatrix;
	XMFLOAT4X4 viewMatrix;
	XMFLOAT4X4 projectionMatrix;

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;
	POINT currentMousePos;
	POINT difference;

};

