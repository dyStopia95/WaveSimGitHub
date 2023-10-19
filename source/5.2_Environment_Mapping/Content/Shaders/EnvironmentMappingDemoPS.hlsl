cbuffer CBufferPerFrame
{
	float3 AmbientColor;
	float3 EnvironmentColor;
}

cbuffer CBufferPerObject
{
	float ReflectionAmount;
}

Texture2D ColorMap : register(t0);
TextureCube EnvironmentMap : register(t1);
SamplerState TextureSampler;

struct VS_OUTPUT
{
	float4 Position: SV_Position;
	float2 TextureCoordinates : TEXCOORD;
	float3 ReflectionVector : REFLECTION;
};

float4 main(VS_OUTPUT IN) : SV_TARGET
{
	float4 OUT = (float4)0;

	float4 color = ColorMap.Sample(TextureSampler, IN.TextureCoordinates);
	float3 ambient = AmbientColor * color.rgb;
	float3 environment = EnvironmentColor * EnvironmentMap.Sample(TextureSampler, IN.ReflectionVector).rgb;

	OUT.rgb = saturate(lerp(ambient, environment, ReflectionAmount));
	OUT.a = color.a;

	return OUT;
}