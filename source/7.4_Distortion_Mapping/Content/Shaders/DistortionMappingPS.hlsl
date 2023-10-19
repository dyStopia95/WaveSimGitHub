static const float ZeroCorrection = 0.5f / 255.0f;

cbuffer CBufferPerObject
{
	float DisplacementScale;
}

Texture2D ColorMap : register(t0);
Texture2D DistortionMap : register(t1);
SamplerState TextureSampler;

struct VS_OUTPUT
{
	float4 Position : SV_Position;
	float2 TextureCoordinates : TEXCOORD;
};

float4 main(VS_OUTPUT IN) : SV_TARGET
{
	float2 displacement = DistortionMap.Sample(TextureSampler, IN.TextureCoordinates).xy - 0.5 + ZeroCorrection;
	return ColorMap.Sample(TextureSampler, IN.TextureCoordinates + (DisplacementScale * displacement));
}