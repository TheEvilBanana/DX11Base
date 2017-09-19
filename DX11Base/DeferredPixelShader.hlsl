
Texture2D textureSRV : register(t0);
Texture2D normalMapSRV : register(t1);

SamplerState basicSampler : register(s0);

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 normal		: NORMALWS;
	float3 tangent		: TANGENTWS;
	float3 worldPos		: POSITIONWS;
	float2 uv			: TEXCOORD;
};

struct PSOut
{
	float4 Position		: SV_Target0;
	float4 Normal		: SV_Target1;
	float4 Diffuse		: SV_Target2;
};

PSOut main(VertexToPixel input) 
{
	PSOut output;

	float3 diffuse = textureSRV.Sample(basicSampler, input.uv).rgb;

	float3 N = input.normal;
	float3 T = normalize(input.tangent - N * dot(input.tangent, N));
	float3 bitangent = cross(T, N);

	float3x3 tangentFrame = float3x3(normalize(input.tangent), normalize(bitangent), normalize(input.normal));
	
	float3 normal = normalMapSRV.Sample(basicSampler, input.uv).rgb;
	normal = normalize(normal * 2.0f - 1.0f);

	float3 normalWS = mul(normal, tangentFrame);

	output.Position = float4(input.worldPos, 1.0f);
	output.Normal = float4(normalWS, 1.0f);
	output.Diffuse = float4(diffuse, 1.0f);

	return output;
}