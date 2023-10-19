cbuffer CBufferPerFrame
{
	float3 LightPosition;
	float LightRadius;
}

cbuffer CBufferPerObject
{
	float4x4 WorldViewProjection;
	float4x4 World;
	float4x4 ProjectiveTextureMatrix;
}

struct VS_INPUT
{
	float4 ObjectPosition : POSITION;
	float2 TextureCoordinates : TEXCOORD;
	float3 Normal : NORMAL;
};

struct VS_OUTPUT
{
	float4 Position: SV_Position;
	float3 WorldPosition : WORLDPOS;
	float2 TextureCoordinates : TEXCOORD;
	float3 Normal : NORMAL;
	float Attenuation : ATTENUATION;
	float4 ShadowTextureCoordinates : PROJCOORD;
};

VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;

	OUT.Position = mul(IN.ObjectPosition, WorldViewProjection);
	OUT.WorldPosition = mul(IN.ObjectPosition, World).xyz;
	OUT.TextureCoordinates = IN.TextureCoordinates;
	OUT.Normal = normalize(mul(float4(IN.Normal, 0), World).xyz);

	float3 lightDirection = LightPosition - OUT.WorldPosition;
	OUT.Attenuation = saturate(1.0f - (length(lightDirection) / LightRadius));

	OUT.ShadowTextureCoordinates = mul(IN.ObjectPosition, ProjectiveTextureMatrix);

	return OUT;
}