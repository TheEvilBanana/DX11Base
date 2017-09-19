
// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

// Textures and such
Texture2D Texture		: register(t0);
SamplerState Sampler	: register(s0);

// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	// Total color before dividing
	float4 totalColor = Texture.Sample(Sampler, input.uv);
	// Return the average color
	return totalColor;

}