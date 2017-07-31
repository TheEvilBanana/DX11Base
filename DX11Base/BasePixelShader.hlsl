
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

cbuffer ExternalData : register(b0) {
	DirectionalLight dirLight_1;
	AmbientLight ambientLight;
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

	float4 lightAmount1 = saturate(dot(input.normal, -normalize(dirLight_1.direction)));

	float4 surfaceColor = textureSRV.Sample(basicSampler, input.uv);
	
	return surfaceColor;

	float4 totalLight = (dirLight_1.diffuseColor * lightAmount1 * surfaceColor) + (surfaceColor * ambientLight.ambientColor);

	return totalLight;
}