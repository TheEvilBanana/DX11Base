cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
};

struct VertexShaderInput
{
	float3 position		: POSITION;
};

struct VertexToPixel
{
	float4 position		: SV_POSITION;
};

VertexToPixel main(in VertexShaderInput input)
{
	VertexToPixel output;

	matrix worldViewProj = mul(mul(world, view), projection);
	output.position = mul(float4(input.position, 1.0f), worldViewProj);
	//output.position = float4(input.position, 1.0f);

	return output;
}