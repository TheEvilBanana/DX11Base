Texture2D positionGB	: register(t0);
Texture2D normalGB		: register(t1);
Texture2D diffuseGB		: register(t2);

cbuffer ExternalData : register(b0)
{
	float3 lightColor;
	float3 lightDir;
}

struct VertexToPixel
{
	float4 position		: SV_POSITION;
};

float4 main(in VertexToPixel input) : SV_TARGET
{
	int3 sampleIndices = int3(input.position.xy, 0);

	float3 normal = normalGB.Load(sampleIndices).xyz;

	float3 position = positionGB.Load(sampleIndices).xyz;

	float3 diffuse = diffuseGB.Load(sampleIndices).xyz;

	float3 L = -lightDir;

	float lightAmountDL = saturate(dot(normal, L));
	float3 color = lightColor * lightAmountDL * diffuse;

	return float4(color, 1.0f);
}