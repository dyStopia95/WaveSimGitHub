struct PointLight
{
	float3 Position;
	float Radius;
};

#define LightCount 4
cbuffer CBufferPerFrame
{
	PointLight Lights[LightCount];
}

cbuffer CBufferPerObject
{
	float4x4 WorldViewProjection;
	float4x4 World;
}

struct VS_INPUT
{
	float4 ObjectPosition: POSITION;
	float2 TextureCoordinates : TEXCOORD;
	float3 Normal : NORMAL;
};

struct VS_OUTPUT
{
	float4 Position: SV_Position;
	float3 WorldPosition : WORLDPOS;	
	float2 TextureCoordinates : TEXCOORD;
	float3 Normal : NORMAL;
	float LightAttenuation[LightCount]: ATTENUATION;
};

VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;

	OUT.Position = mul(IN.ObjectPosition, WorldViewProjection);
	OUT.WorldPosition = mul(IN.ObjectPosition, World).xyz;
	OUT.TextureCoordinates = IN.TextureCoordinates;
	OUT.Normal = normalize(mul(float4(IN.Normal, 0), World).xyz);

	[unroll]
	for (int i = 0; i < LightCount; ++i)
	{
		float3 lightDirection = Lights[i].Position - OUT.WorldPosition;
		OUT.LightAttenuation[i] = saturate(1.0f - (length(lightDirection) / Lights[i].Radius));
	}	

	return OUT;
}