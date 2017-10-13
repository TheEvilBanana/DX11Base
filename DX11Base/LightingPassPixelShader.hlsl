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

	float3 L = lightPos - position;
	float dist = length(L);

	if (dist > 2.0f)
	{
		return float4(0.0f, 0.0f, 0.0f, 0.0f);
	}

	L /= dist;

	float att = max(0.0f, 1.0f - (dist / 2.0f));

	float lightAmount = saturate(dot(normal, L));
	float3 color = lightAmount * lightColor * att;

	//Specular calc
	float3 V = cameraPosition - position;
	float3 H = normalize(L + V);
	float specular = pow(saturate(dot(normal, H)), 10) * att;

	float3 finalDiffuse = color * diffuse;
	float3 finalSpecular = specular * diffuse * att;

	float4 totalColor = float4(finalDiffuse + finalSpecular, 1.0f);
	return totalColor;
}