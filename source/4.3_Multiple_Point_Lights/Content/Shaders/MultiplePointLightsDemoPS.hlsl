struct LightContributionData
{
	float3 Color;
	float3 Normal;
	float3 ViewDirection;
	float3 LightColor;
	float3 LightDirection;
	float Attenuation;
	float3 SpecularColor;
	float SpecularPower;
	float SpecularClamp;
};

float3 GetLightContribution(LightContributionData IN)
{
	float n_dot_l = dot(IN.Normal, IN.LightDirection);
	float3 halfVector = normalize(IN.LightDirection + IN.ViewDirection);
	float n_dot_h = dot(IN.Normal, halfVector);

	float2 lightCoefficients = lit(n_dot_l, n_dot_h, IN.SpecularPower).yz;
	float3 diffuse = IN.Color * lightCoefficients.x * IN.LightColor * IN.Attenuation;
	float3 specular = min(lightCoefficients.y, IN.SpecularClamp) * IN.SpecularColor * IN.Attenuation;

	return (diffuse + specular);
}

struct PointLight
{
	float3 Position;
	float3 Color;
};

#define LightCount 4
cbuffer CBufferPerFrame
{
	PointLight Lights[LightCount];
	float3 CameraPosition;
	float3 AmbientColor;
};

cbuffer CBufferPerObject
{
	float SpecularPower;
}

Texture2D ColorMap;
Texture2D SpecularMap;
SamplerState TextureSampler;

struct VS_OUTPUT
{
	float4 Position: SV_Position;
	float3 WorldPosition : WORLDPOS;
	float2 TextureCoordinates : TEXCOORD;
	float3 Normal : NORMAL;
	float LightAttenuation[LightCount]: ATTENUATION;
};

float4 main(VS_OUTPUT IN) : SV_TARGET
{
	float4 color = ColorMap.Sample(TextureSampler, IN.TextureCoordinates);
	float3 ambient = color.rgb * AmbientColor;
	float specularClamp = SpecularMap.Sample(TextureSampler, IN.TextureCoordinates).x;

	LightContributionData lightContributionData;
	lightContributionData.Color = color.rgb;
	lightContributionData.Normal = normalize(IN.Normal);
	lightContributionData.ViewDirection = normalize(CameraPosition - IN.WorldPosition);	
	lightContributionData.SpecularPower = SpecularPower;
	lightContributionData.SpecularClamp = specularClamp;

	float3 totalLightContribution = (float3)0;

	for (int i = 0; i < LightCount; i++)
	{
		lightContributionData.LightColor = Lights[i].Color;
		lightContributionData.SpecularColor = Lights[i].Color;
		lightContributionData.LightDirection = normalize(Lights[i].Position - IN.WorldPosition);
		lightContributionData.Attenuation = IN.LightAttenuation[i];
		totalLightContribution += GetLightContribution(lightContributionData);
	}

	return float4(saturate(ambient + totalLightContribution), color.a);
}