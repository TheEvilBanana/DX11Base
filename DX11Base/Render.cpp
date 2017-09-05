#include "Render.h"



Render::Render()
{
}


Render::~Render()
{
}


void Render::RenderProcess(GameEntity* &gameEntity, ID3D11Buffer* &vertexBuffer, ID3D11Buffer* &indexBuffer, SimpleVertexShader* &vertexShader, SimplePixelShader* &pixelShader, Camera* &camera, ID3D11DeviceContext* &context)
{
	SetLights();

	vertexBuffer = gameEntity->GetMesh()->GetVertexBuffer();
	indexBuffer = gameEntity->GetMesh()->GetIndexBuffer();

	vertexShader->SetMatrix4x4("world", *gameEntity->GetWorldMatrix());
	vertexShader->SetMatrix4x4("view", camera->GetView());
	vertexShader->SetMatrix4x4("projection", camera->GetProjection());

	vertexShader->CopyAllBufferData();
	vertexShader->SetShader();

	pixelShader->SetData("dirLight_1", &dirLight_1, sizeof(DirectionalLight));
	pixelShader->SetData("ambientLight", &ambientLight, sizeof(AmbientLight));
	pixelShader->SetData("pointLight", &pointLight, sizeof(PointLight));
	pixelShader->SetData("spotLight", &spotLight, sizeof(SpotLight));
	pixelShader->SetFloat3("cameraPosition", camera->GetPosition());

	pixelShader->SetShaderResourceView("textureSRV", gameEntity->GetMaterial()->GetMaterialSRV());
	pixelShader->SetShaderResourceView("normalMapSRV", gameEntity->GetMaterial()->GetNormalSRV());
	pixelShader->SetSamplerState("basicSampler", gameEntity->GetMaterial()->GetMaterialSampler());

	pixelShader->CopyAllBufferData();
	pixelShader->SetShader();

	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	context->DrawIndexed(gameEntity->GetMesh()->GetIndexCount(), 0, 0);
}

void Render::RenderSkyBox(Mesh* &mesh, ID3D11Buffer* &vertexBuffer, ID3D11Buffer* &indexBuffer, SimpleVertexShader* &vertexShader, SimplePixelShader* &pixelShader, Camera* &camera, ID3D11DeviceContext* &context, ID3D11RasterizerState* &rasterizerState, ID3D11DepthStencilState* &depthState, ID3D11ShaderResourceView* &SRV)
{
	vertexBuffer = mesh->GetVertexBuffer();
	indexBuffer = mesh->GetIndexBuffer();
	
	vertexShader->SetMatrix4x4("view", camera->GetView());
	vertexShader->SetMatrix4x4("projection", camera->GetProjection());

	vertexShader->CopyAllBufferData();
	vertexShader->SetShader();

	pixelShader->SetShaderResourceView("Sky", SRV);

	pixelShader->CopyAllBufferData();
	pixelShader->SetShader();

	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	context->RSSetState(rasterizerState);
	context->OMSetDepthStencilState(depthState, 0);

	context->DrawIndexed(mesh->GetIndexCount(), 0, 0);

	// Reset the render states we've changed
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);
}

void Render::SetLights()
{
	dirLight_1.SetLightValues(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(10.0f, 0.0f, 0.0f));
	ambientLight.SetLightValues(XMFLOAT4(0.2f, 0.0f, 0.0f, 1.0f));
	pointLight.SetLightValues(XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT3(2.0f, 0.0f, 0.0f), 7.0f, XMFLOAT3(0.0f, 1.0f, 0.0f), 0.0f);
	spotLight.SetLightValues(XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), 10.0f, XMFLOAT3(0.0f, -1.0f, 0.0f), 5.0f, 
									XMFLOAT3(0.0f, 1.0f, 0.0f), 0.0f);
}
