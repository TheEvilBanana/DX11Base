//struct VertexShaderInput
//{
//	float3 position		: POSITION;
//};

struct VertexToPixel
{
	float4 position		: SV_POSITION;
};

VertexToPixel main(uint id : SV_VertexId)
{
	VertexToPixel output;

	float2 Tex = float2(id % 2, (id % 4) >> 1);
	output.position = float4((Tex.x - 0.5f) * 2, -(Tex.y - 0.5f) * 2, 0, 1);

	return output;
}