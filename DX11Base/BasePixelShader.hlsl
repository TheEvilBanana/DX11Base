
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
	float range;
	float3 attenuate;
	float pad;
};

struct SpotLight {
	float4 diffuseColor;
	float3 position;
	float range;
	float3 direction;
	float spot;
	float3 attenuate;
	float pad;
};

cbuffer ExternalData : register(b0) {
	DirectionalLight dirLight_1;
	AmbientLight ambientLight;
	PointLight pointLight;
	SpotLight spotLight;
	float3 cameraPosition;
};

struct VertexToPixel
{

	float4 position		: SV_POSITION;
	float3 normal       : NORMAL;       // Normal co-ordinates
	float3 tangent		: TANGENT;
	float3 worldPos		: POSITION;
	float2 uv           : TEXCOORD;     // UV co-ordinates
};

void ComputeDirectionalLight(VertexToPixel input, DirectionalLight dirLight, float4 surfaceColor, out float4 diffuse)
{
	float lightAmountDL = saturate(dot(input.normal, -normalize(dirLight.direction)));
	float4 directionalL = dirLight_1.diffuseColor * lightAmountDL * surfaceColor;

	diffuse = directionalL;
}

void ComputePointLight(VertexToPixel input, PointLight pointLight, float4 surfaceColor, out float4 diffuse, out float4 specular)
{
	//Point light
	//float3 dirToPointLight = normalize(pointLight.position - input.worldPos);
	float3 dirToPointLight = pointLight.position - input.worldPos;
	float dist = length(dirToPointLight);
	//if (dist > pointLight.range)
	//{
	//	//pointL = (0.0f, 0.0f, 0.0f, 0.0f);
	//	diffuse = (0.0f, 0.0f, 0.0f, 0.0f);
	//	specular = (0.0f, 0.0f, 0.0f, 0.0f);

	//	return;
	//}
	
	dirToPointLight /= dist;
	float lightAmountPL = saturate(dot(input.normal, dirToPointLight));
	float4 pointL = lightAmountPL * pointLight.diffuseColor;
	float att = 1.0f / dot(pointLight.attenuate, float3(1.0f, dist, dist*dist));
	pointL *= att;
	//Point specular
	float3 toCamera = normalize(cameraPosition - input.worldPos);
	float3 refl = reflect(-dirToPointLight, input.normal);
	float specularPL = pow(saturate(dot(refl, toCamera)), 8);
	specular *= att;
	diffuse = pointL;
	specular = specularPL;
}

void ComputeSpotLight(VertexToPixel input, SpotLight spotLight, float4 surfaceColor, out float4 diffuse, out float4 specular)
{
	//Spot light
	float3 dirToSpotLight = spotLight.position - input.worldPos;
	float distSL = length(dirToSpotLight);
	dirToSpotLight /= distSL;
	float lightAmountSL = saturate(dot(input.normal, dirToSpotLight));
	float4 spotL = lightAmountSL * spotLight.diffuseColor;
	float spot = pow(max(dot(-dirToSpotLight, spotLight.direction), 0.0f), spotLight.spot);
	float attSL = spot / dot(spotLight.attenuate, float3(1.0f, distSL, distSL*distSL));
	spotL *= attSL;
	diffuse = spotL;
	//Spot specular
	float3 toCameraSL = normalize(cameraPosition - input.worldPos);
	float3 reflSL = reflect(-dirToSpotLight, input.normal);
	float specularSL = pow(saturate(dot(reflSL, toCameraSL)), 8);
	specularSL *= attSL;
	specular = specularSL;
}

float4 main(VertexToPixel input) : SV_TARGET
{
	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);

	float4 D = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 S = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// Read and unpack normal from map
	float3 normalFromMap = normalMapSRV.Sample(basicSampler, input.uv).xyz * 2 - 1;

	// Transform from tangent to world space
	float3 N = input.normal;
	float3 T = normalize(input.tangent - N * dot(input.tangent, N));
	float3 B = cross(T, N);

	float3x3 TBN = float3x3(T, B, N);
	input.normal = normalize(mul(normalFromMap, TBN));

	float4 surfaceColor = textureSRV.Sample(basicSampler, input.uv);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	//Directional light
	/*float lightAmountDL = saturate(dot(input.normal, -normalize(dirLight_1.direction)));
	float4 directionalL = dirLight_1.diffuseColor * lightAmountDL * surfaceColor;
	
	diffuse += directionalL;*/

	ComputeDirectionalLight(input, dirLight_1, surfaceColor, D);
	diffuse += D;
	
	//Ambient light
	float4 ambient = surfaceColor * ambientLight.ambientColor;

	//Point light
	ComputePointLight(input, pointLight, surfaceColor, D, S);
	diffuse += D;
	specular += S;

	//Spot light
	ComputeSpotLight(input, spotLight, surfaceColor, D, S);
	diffuse += D;
	specular += S;
	
	//Total light
	float4 totalLight = diffuse + specular + ambient;
	return totalLight;

}