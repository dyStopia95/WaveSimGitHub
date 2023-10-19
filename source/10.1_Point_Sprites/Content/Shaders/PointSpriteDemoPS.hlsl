Texture2D ColorTexture;
SamplerState ColorSampler;

struct GS_OUTPUT
{
	float4 Position : SV_Position;
	float2 TextureCoordinates : TEXCOORD;
};

float4 main(GS_OUTPUT IN) : SV_TARGET
{
	return ColorTexture.Sample(ColorSampler, IN.TextureCoordinates);
}