#pragma once

#include "GameEntity.h"
#include "Camera.h"
#include "Lights.h"

class Render
{
public:
	Render();
	~Render();

	void SetVertexBuffer(GameEntity* &gameEntity, ID3D11Buffer* &vertexBuffer);
	void SetIndexBuffer(GameEntity* &gameEntity, ID3D11Buffer* &indexBuffer);
	void SetVertexShader(SimpleVertexShader* &vertexShader, GameEntity* &gameEntity, Camera* &camera);
	void SetPixelShader(SimplePixelShader* &pixelShader, GameEntity* &gameEntity, Camera* &camera);
	void RenderProcess(GameEntity* &gameEntity, ID3D11Buffer* &vertexBuffer, ID3D11Buffer* &indexBuffer, SimpleVertexShader* &vertexShader, SimplePixelShader* &pixelShader, Camera* &camera, ID3D11DeviceContext* &context);

private:

	void SetLights();

	DirectionalLight dirLight_1;
	AmbientLight ambientLight;
	PointLight pointLight;
};

