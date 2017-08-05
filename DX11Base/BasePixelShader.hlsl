
Texture2D textureSRV : register(t0);
Texture2D normalMapSRV : register(t1);
SamplerState basicSampler : register(s0);

struct DirectionalLight {
	float4 diffuseColor;
	float3 direction;
};

struct AmbientLight {
	float4 ambientColor;
};

struct PointLight {
	float4 diffuseColor;
	float3 position;
};

cbuffer ExternalData : register(b0) {
	DirectionalLight dirLight_1;
	AmbientLight ambientLight;
	PointLight pointLight;
};

struct VertexToPixel
{

	float4 position		: SV_POSITION;
	float3 normal       : NORMAL;       // Normal co-ordinates
	float3 tangent		: TANGENT;
	float3 worldPos		: POSITION;
	float2 uv           : TEXCOORD;     // UV co-ordinates
};


float4 main(VertexToPixel input) : SV_TARGET
{
	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);

	// Read and unpack normal from map
	float3 normalFromMap = normalMapSRV.Sample(basicSampler, input.uv).xyz * 2 - 1;

	// Transform from tangent to world space
	float3 N = input.normal;
	float3 T = normalize(input.tangent - N * dot(input.tangent, N));
	float3 B = cross(T, N);

	float3x3 TBN = float3x3(T, B, N);
	input.normal = normalize(mul(normalFromMap, TBN));

	float4 surfaceColor = textureSRV.Sample(basicSampler, input.uv);

	float4 lightAmountDL = saturate(dot(input.normal, -normalize(dirLight_1.direction)));

	float3 dirToPointLight = normalize(pointLight.position - input.worldPos);
	float lightAmountPL = saturate(dot(input.normal, dirToPointLight));

	float4 totalLight = (dirLight_1.diffuseColor * lightAmountDL * surfaceColor) + (surfaceColor * ambientLight.ambientColor) +
							(lightAmountPL * pointLight.diffuseColor);

	return totalLight;
}