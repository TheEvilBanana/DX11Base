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

	pixelShader->SetData("pointLight1", &pointLight1, sizeof(PointLight));
	pixelShader->SetData("pointLight2", &pointLight2, sizeof(PointLight));
	pixelShader->SetData("pointLight3", &pointLight3, sizeof(PointLight));
	pixelShader->SetData("pointLight4", &pointLight4, sizeof(PointLight));

	pixelShader->SetData("outerPointLight1", &outerPointLight1, sizeof(PointLight));
	pixelShader->SetData("outerPointLight2", &outerPointLight2, sizeof(PointLight));
	pixelShader->SetData("outerPointLight3", &outerPointLight3, sizeof(PointLight));
	pixelShader->SetData("outerPointLight4", &outerPointLight4, sizeof(PointLight));
	pixelShader->SetData("outerPointLight5", &outerPointLight5, sizeof(PointLight));
	pixelShader->SetData("outerPointLight6", &outerPointLight6, sizeof(PointLight));
	pixelShader->SetData("outerPointLight7", &outerPointLight7, sizeof(PointLight));
	pixelShader->SetData("outerPointLight8", &outerPointLight8, sizeof(PointLight));

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

void Render::RenderGBuffer(GameEntity* &gameEntity, ID3D11Buffer* &vertexBuffer, ID3D11Buffer* &indexBuffer, SimpleVertexShader* &vertexShader, SimplePixelShader* &pixelShader, Camera* &camera, ID3D11DeviceContext* &context)
{
	vertexBuffer = gameEntity->GetMesh()->GetVertexBuffer();
	indexBuffer = gameEntity->GetMesh()->GetIndexBuffer();

	vertexShader->SetMatrix4x4("world", *gameEntity->GetWorldMatrix());
	vertexShader->SetMatrix4x4("view", camera->GetView());
	vertexShader->SetMatrix4x4("projection", camera->GetProjection());

	vertexShader->CopyAllBufferData();
	vertexShader->SetShader();

	pixelShader->SetShaderResourceView("textureSRV", gameEntity->GetMaterial()->GetMaterialSRV());
	pixelShader->SetShaderResourceView("normalMapSRV", gameEntity->GetMaterial()->GetNormalSRV());
	pixelShader->SetSamplerState("basicSampler", gameEntity->GetMaterial()->GetMaterialSampler());

	pixelShader->CopyAllBufferData();
	pixelShader->SetShader();

	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	context->DrawIndexed(gameEntity->GetMesh()->GetIndexCount(), 0, 0);
}

void Render::RenderLights(GameEntity* &gameEntity, ID3D11Buffer* &vertexBuffer, ID3D11Buffer* &indexBuffer, SimpleVertexShader* &vertexShader, SimplePixelShader* &pixelShader, Camera* &camera, ID3D11DeviceContext* &context, ID3D11SamplerState* &sampler, ID3D11ShaderResourceView* &positionGBuffer, ID3D11ShaderResourceView* &normalGBuffer, ID3D11ShaderResourceView* &diffuseGBuffer)
{
	vertexBuffer = gameEntity->GetMesh()->GetVertexBuffer();
	indexBuffer = gameEntity->GetMesh()->GetIndexBuffer();

	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	vertexShader->SetMatrix4x4("world", *gameEntity->GetWorldMatrix());
	vertexShader->SetMatrix4x4("view", camera->GetView());
	vertexShader->SetMatrix4x4("projection", camera->GetProjection());

	vertexShader->CopyAllBufferData();
	vertexShader->SetShader();

	pixelShader->SetShaderResourceView("positionGB", positionGBuffer);
	pixelShader->SetShaderResourceView("normalGB", normalGBuffer);
	pixelShader->SetShaderResourceView("diffuseGB", diffuseGBuffer);

	pixelShader->SetSamplerState("basicSampler", sampler);

	pixelShader->SetFloat3("cameraPosition", camera->GetPosition());

	pixelShader->SetFloat3("lightColor", gameEntity->GetLightColor());
	pixelShader->SetFloat3("lightPos", gameEntity->GetPosition());

	pixelShader->CopyAllBufferData();
	pixelShader->SetShader();

	context->DrawIndexed(gameEntity->GetMesh()->GetIndexCount(), 0, 0);
}

void Render::SetLights()
{
	dirLight_1.SetLightValues(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(10.0f, 0.0f, 0.0f), 0.0f);
	ambientLight.SetLightValues(XMFLOAT4(0.2f, 0.0f, 0.0f, 1.0f));
	pointLight1.SetLightValues(XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT3(2.0f, 0.0f, 0.0f), 7.0f, XMFLOAT3(0.0f, 1.0f, 0.0f), 0.0f);
	pointLight2.SetLightValues(XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), XMFLOAT3(-2.0f, 0.0f, 0.0f), 7.0f, XMFLOAT3(0.0f, 1.0f, 0.0f), 0.0f);
	pointLight3.SetLightValues(XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 2.0f), 7.0f, XMFLOAT3(0.0f, 1.0f, 0.0f), 0.0f);
	pointLight4.SetLightValues(XMFLOAT4(0.6f, 0.0f, 0.4f, 1.0f), XMFLOAT3(0.0f, 0.0f, -2.0f), 7.0f, XMFLOAT3(0.0f, 1.0f, 0.0f), 0.0f);
	outerPointLight1.SetLightValues(XMFLOAT4(0.1f, 0.9f, 0.1f, 1.0f), XMFLOAT3(0.0f, 0.0f, -4.0f), 7.0f, XMFLOAT3(0.0f, 1.0f, 0.0f), 0.0f);
	outerPointLight2.SetLightValues(XMFLOAT4(0.8f, 0.2f, 0.0f, 1.0f), XMFLOAT3(4.0f, 0.0f, -4.0f), 7.0f, XMFLOAT3(0.0f, 1.0f, 0.0f), 0.0f);
	outerPointLight3.SetLightValues(XMFLOAT4(0.0f, 0.3f, 0.8f, 1.0f), XMFLOAT3(4.0f, 0.0f, 0.0f), 7.0f, XMFLOAT3(0.0f, 1.0f, 0.0f), 0.0f);
	outerPointLight4.SetLightValues(XMFLOAT4(0.9f, 0.3f, 0.3f, 1.0f), XMFLOAT3(4.0f, 0.0f, 4.0f), 7.0f, XMFLOAT3(0.0f, 1.0f, 0.0f), 0.0f);
	outerPointLight5.SetLightValues(XMFLOAT4(0.2f, 0.7f, 0.3f, 1.0f), XMFLOAT3(0.0f, 0.0f, 4.0f), 7.0f, XMFLOAT3(0.0f, 1.0f, 0.0f), 0.0f);
	outerPointLight6.SetLightValues(XMFLOAT4(0.1f, 0.0f, 0.8f, 1.0f), XMFLOAT3(-4.0f, 0.0f, 4.0f), 7.0f, XMFLOAT3(0.0f, 1.0f, 0.0f), 0.0f);
	outerPointLight7.SetLightValues(XMFLOAT4(0.9f, 0.0f, 0.3f, 1.0f), XMFLOAT3(-4.0f, 0.0f, 0.0f), 7.0f, XMFLOAT3(0.0f, 1.0f, 0.0f), 0.0f);
	outerPointLight8.SetLightValues(XMFLOAT4(0.0f, 0.8f, 0.3f, 1.0f), XMFLOAT3(-4.0f, 0.0f,-4.0f), 7.0f, XMFLOAT3(0.0f, 1.0f, 0.0f), 0.0f);
	spotLight.SetLightValues(XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), 10.0f, XMFLOAT3(0.0f, -1.0f, 0.0f), 5.0f, XMFLOAT3(0.0f, 1.0f, 0.0f), 0.0f);
	
}
