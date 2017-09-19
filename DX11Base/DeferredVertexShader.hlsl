cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
};

struct VertexShaderInput
{
	float3 position		: POSITION;
	float2 uv			: TEXCOORD0;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
};

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 normal		: NORMALWS;
	float3 tangent		: TANGENTWS;
	float3 worldPos		: POSITIONWS;
	float2 uv			: TEXCOORD;
};

VertexToPixel main(VertexShaderInput input)
{
	VertexToPixel output;

	matrix worldViewProj = mul(mul(world, view), projection);
	output.position = mul(float4(input.position, 1.0f), worldViewProj);

	output.worldPos = mul(float4(input.position, 1.0f), world).xyz;
	
	output.normal = normalize(mul(input.normal, (float3x3)world));

	output.tangent = normalize(mul(input.tangent, (float3x3)world));

	output.uv = input.uv;

	return output;
}