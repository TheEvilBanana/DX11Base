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

	output.position = float4(input.position, 1.0f);

	return output;
}