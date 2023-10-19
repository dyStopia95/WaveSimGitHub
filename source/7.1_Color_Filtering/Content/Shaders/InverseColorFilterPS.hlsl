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

	return float4(1 - color.rgb, color.a);
}