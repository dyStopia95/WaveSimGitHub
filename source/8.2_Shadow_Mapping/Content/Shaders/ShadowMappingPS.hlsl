static const float3 ColorWhite = { 1, 1, 1 };
static const float3 ColorBlack = { 0, 0, 0 };
static const float DepthBias = 0.005;

cbuffer CBufferPerFrame
{
	float3 CameraPosition;
	float4 AmbientColor = { 1.0f, 1.0f, 1.0f, 0.0f };
	float3 LightPosition = { 0.0f, 0.0f, 0.0f };
	float4 LightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	float2 ShadowMapSize = { 1024.0f, 1024.0f };
}

Texture2D ColorTexture : register(t0);
Texture2D ShadowMap : register(t1);

SamplerState ColorSampler : register(s0);
SamplerState ShadowMapSampler : register(s1);
SamplerComparisonState PcfShadowMapSampler : register(s2);

struct VS_OUTPUT
{
	float4 Position: SV_Position;
	float3 WorldPosition : WORLDPOS;
	float2 TextureCoordinates : TEXCOORD;
	float3 Normal : NORMAL;
	float Attenuation : ATTENUATION;
	float4 ShadowTextureCoordinates : PROJCOORD;
};

interface IShadowMappingShader
{
	float4 ComputeFinalColor(VS_OUTPUT IN);
};

class PointLightShader
{
	float4 ComputeFinalColor(VS_OUTPUT IN)
	{
		float4 OUT = (float4)0;

		float3 lightDirection = normalize(LightPosition - IN.WorldPosition);
		float3 normal = normalize(IN.Normal);
		float n_dot_l = dot(normal, lightDirection);

		float4 color = ColorTexture.Sample(ColorSampler, IN.TextureCoordinates);

		float3 ambient = AmbientColor.rgb * color.rgb;
		float3 diffuse = LightColor.rgb * saturate(n_dot_l) * color.rgb * IN.Attenuation;

		OUT.rgb = ambient + diffuse;
		OUT.a = 1.0f;

		return OUT;
	}
};

class BasicShadowMappingShader : IShadowMappingShader
{
	float4 ComputeFinalColor(VS_OUTPUT IN)
	{
		PointLightShader pointLightShader;
		float4 OUT = pointLightShader.ComputeFinalColor(IN);

		if (IN.ShadowTextureCoordinates.w >= 0.0f)
		{
			IN.ShadowTextureCoordinates.xyz /= IN.ShadowTextureCoordinates.w;
			float pixelDepth = IN.ShadowTextureCoordinates.z;
			float sampledDepth = ShadowMap.Sample(ShadowMapSampler, IN.ShadowTextureCoordinates.xy).x + DepthBias;

			// Shadow applied in a boolean fashion -- either in shadow or not
			float3 shadow = (pixelDepth > sampledDepth ? ColorBlack : ColorWhite);
			OUT.rgb *= shadow;
		}

		return OUT;
	}
};

class ManualPcfShadowMappingShader : IShadowMappingShader
{
	float4 ComputeFinalColor(VS_OUTPUT IN)
	{
		PointLightShader pointLightShader;
		float4 OUT = pointLightShader.ComputeFinalColor(IN);

		if (IN.ShadowTextureCoordinates.w >= 0.0f)
		{
			IN.ShadowTextureCoordinates.xyz /= IN.ShadowTextureCoordinates.w;

			float2 texelSize = 1.0f / ShadowMapSize;
			float4 sampledDepth;
			sampledDepth.x = ShadowMap.Sample(ShadowMapSampler, IN.ShadowTextureCoordinates.xy).x;
			sampledDepth.y = ShadowMap.Sample(ShadowMapSampler, IN.ShadowTextureCoordinates.xy + float2(texelSize.x, 0)).x;
			sampledDepth.z = ShadowMap.Sample(ShadowMapSampler, IN.ShadowTextureCoordinates.xy + float2(0, texelSize.y)).x;
			sampledDepth.w = ShadowMap.Sample(ShadowMapSampler, IN.ShadowTextureCoordinates.xy + float2(texelSize.x, texelSize.y)).x;
			sampledDepth += DepthBias;

			float pixelDepth = IN.ShadowTextureCoordinates.z;
			float4 shadowFactor = (pixelDepth > sampledDepth ? 0.0f : 1.0f);
			float2 lerpValues = frac(IN.ShadowTextureCoordinates.xy * ShadowMapSize);
			float shadow = lerp(lerp(shadowFactor.x, shadowFactor.y, lerpValues.x), lerp(shadowFactor.z, shadowFactor.w, lerpValues.x), lerpValues.y);
			OUT.rgb *= shadow;
		}

		return OUT;
	}
};

class PcfShadowMappingShader : IShadowMappingShader
{
	float4 ComputeFinalColor(VS_OUTPUT IN)
	{
		PointLightShader pointLightShader;
		float4 OUT = pointLightShader.ComputeFinalColor(IN);

		IN.ShadowTextureCoordinates.xyz /= IN.ShadowTextureCoordinates.w;
		float pixelDepth = IN.ShadowTextureCoordinates.z;

		float shadow = ShadowMap.SampleCmpLevelZero(PcfShadowMapSampler, IN.ShadowTextureCoordinates.xy, pixelDepth).x;
		OUT *= shadow;

		return OUT;
	}
};

IShadowMappingShader ShadowMappingShader;

float4 main(VS_OUTPUT IN) : SV_TARGET
{
	return ShadowMappingShader.ComputeFinalColor(IN);
}