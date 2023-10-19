static const float3 GrayScaleIntensity = { 0.299f, 0.587f, 0.114f };

Texture2D ColorMap;
SamplerState TextureSampler;

struct VS_OUTPUT
{
	float4 Position : SV_Position;
	float2 TextureCoordinates : TEXCOORD;
};

float4 main(VS_OUTPUT IN) : SV_TARGET
{
	float4 color = ColorMap.Sample(TextureSampler, IN.TextureCoordinates);
	float intensity = dot(color.rgb, GrayScaleIntensity);

	return float4(intensity.rrr, color.a);
}