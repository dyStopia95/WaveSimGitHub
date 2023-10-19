cbuffer CBufferPerObject
{
	float4x4 WorldViewProjection;
	float4x4 World;
	float DisplacementScale;
}

Texture2D DisplacementMap;
SamplerState TextureSampler;

struct VS_INPUT
{
	float4 ObjectPosition : POSITION;
	float2 TextureCoordinates : TEXCOORD;
	float3 Normal : NORMAL;
};

struct VS_OUTPUT
{
	float4 Position: SV_Position;
	float2 TextureCoordinates : TEXCOORD;
	float3 Normal : NORMAL;
};

VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;

	float displacement = DisplacementMap.SampleLevel(TextureSampler, IN.TextureCoordinates, 0).x;
	IN.ObjectPosition.xyz +=  IN.Normal * DisplacementScale * displacement;

	OUT.Position = mul(IN.ObjectPosition, WorldViewProjection);
	OUT.TextureCoordinates = IN.TextureCoordinates;
	OUT.Normal = normalize(mul(float4(IN.Normal, 0), World).xyz);

	return OUT;
}
