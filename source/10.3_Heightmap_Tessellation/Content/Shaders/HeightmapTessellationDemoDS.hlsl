cbuffer CBufferPerObject
{
	float4x4 WorldViewProjection;
	float DisplacementScale;
}

struct HS_CONSTANT_OUTPUT
{
	float EdgeFactors[4] : SV_TessFactor;
	float InsideFactors[2] : SV_InsideTessFactor;
};

struct HS_OUTPUT
{
	float4 ObjectPosition : SV_Position;
	float2 TextureCoordinates: TEXCOORD;
};

struct DS_OUTPUT
{
	float4 Position : SV_Position;
};

Texture2D Heightmap;
SamplerState HeightmapSampler;

[domain("quad")]
DS_OUTPUT main(HS_CONSTANT_OUTPUT IN, float2 uv : SV_DomainLocation, const OutputPatch<HS_OUTPUT, 4> patch)
{
	DS_OUTPUT OUT;

	float4 v0 = lerp(patch[0].ObjectPosition, patch[1].ObjectPosition, uv.x);
	float4 v1 = lerp(patch[2].ObjectPosition, patch[3].ObjectPosition, uv.x);
	float4 objectPosition = lerp(v0, v1, uv.y);

	float2 texCoord0 = lerp(patch[0].TextureCoordinates, patch[1].TextureCoordinates, uv.x);
	float2 texCoord1 = lerp(patch[2].TextureCoordinates, patch[3].TextureCoordinates, uv.x);
	float2 textureCoordinate = lerp(texCoord0, texCoord1, uv.y);

	objectPosition.y = (2 * Heightmap.SampleLevel(HeightmapSampler, textureCoordinate, 0).x - 1) * DisplacementScale;
	OUT.Position = mul(float4(objectPosition.xyz, 1.0f), WorldViewProjection);

	return OUT;
}