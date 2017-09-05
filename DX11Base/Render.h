#pragma once

#include "GameEntity.h"
#include "Camera.h"
#include "Lights.h"

class Render
{
public:
	Render();
	~Render();

	void RenderProcess(GameEntity* &gameEntity, ID3D11Buffer* &vertexBuffer, ID3D11Buffer* &indexBuffer, SimpleVertexShader* &vertexShader, SimplePixelShader* &pixelShader, Camera* &camera, ID3D11DeviceContext* &context);
	void RenderSkyBox(Mesh* &mesh, ID3D11Buffer* &vertexBuffer, ID3D11Buffer* &indexBuffer, SimpleVertexShader* &vertexShader, SimplePixelShader* &pixelShader, Camera* &camera, ID3D11DeviceContext* &context, ID3D11RasterizerState* &rasterizerState, ID3D11DepthStencilState* &depthState, ID3D11ShaderResourceView* &SRV);

private:
	
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	void SetLights();

	DirectionalLight dirLight_1;
	AmbientLight ambientLight;
	PointLight pointLight1;
	PointLight pointLight2;
	PointLight pointLight3;
	PointLight pointLight4;
	SpotLight spotLight;
};

