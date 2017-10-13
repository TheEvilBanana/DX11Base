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
	float2 uv = float2(
		(id << 1) & 2,  // id % 2 * 2
		id & 2);

	// Adjust the position based on the UV
	output.position = float4(uv, 0, 1);
	output.position.x = output.position.x * 2 - 1;
	output.position.y = output.position.y * -2 + 1;

	return output;
}