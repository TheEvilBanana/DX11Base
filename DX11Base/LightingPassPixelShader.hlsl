Texture2D positionGB	: register(t0);
Texture2D normalGB		: register(t1);
Texture2D diffuseGB		: register(t2);

SamplerState basicSampler : register(s0);

cbuffer ExternalData : register(b0)
{
	float3 cameraPosition;
	float3 lightColor;
	float3 lightPos;
}

struct VertexToPixel
{
	float4 position		: SV_POSITION;
};

float4 main( in VertexToPixel input) : SV_TARGET
{
	int3 sampleIndices = int3(input.position.xy, 0);

	float3 normal = normalGB.Load(sampleIndices).xyz;

	float3 position = positionGB.Load(sampleIndices).xyz;

	float3 diffuse = diffuseGB.Load(sampleIndices).xyz;

	float3 dirToPointLight = normalize(lightPos - position);
	float lightAmount = saturate(dot(normal, dirToPointLight));

	float3 totalColor = lightAmount * lightColor * diffuse;
	return float4(totalColor, 1.0f);
}