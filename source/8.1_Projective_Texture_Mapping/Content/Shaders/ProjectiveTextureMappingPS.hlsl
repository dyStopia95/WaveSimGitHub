static const float3 ColorWhite = { 1, 1, 1 };

cbuffer CBufferPerFrame
{
	float DepthBias;
	float3 CameraPosition;
	float3 AmbientColor;
	float3 LightPosition;
	float3 LightColor;	
}

Texture2D ColorMap : register(t0);
Texture2D ProjectedMap : register(t1);
Texture2D DepthMap : register(t2);

SamplerState ColorMapSampler : register(s0);
SamplerState ProjectedMapSampler : register(s1);
SamplerState DepthMapSampler : register(s2);

struct VS_OUTPUT
{
	float4 Position: SV_Position;
	float3 WorldPosition : WORLDPOS;
	float2 TextureCoordinates : TEXCOORD;
	float3 Normal : NORMAL;
	float Attenuation : ATTENUATION;
	float4 ProjectedMapCoordinates : PROJCOORD;
};

interface IProjectiveTextureMappingShader
{
	float4 ComputeFinalColor(VS_OUTPUT IN);
};

class PointLightShader
{
	float4 ComputeFinalColor(VS_OUTPUT IN)
	{
		float3 lightDirection = normalize(LightPosition - IN.WorldPosition);
		float3 normal = normalize(IN.Normal);
		float n_dot_l = dot(normal, lightDirection);

		float4 color = ColorMap.Sample(ColorMapSampler, IN.TextureCoordinates);

		float3 ambient = color.rgb * AmbientColor;
		float3 diffuse = color.rgb * saturate(n_dot_l) * LightColor * IN.Attenuation;

		return float4(saturate(ambient + diffuse), color.a);
	}
};

class BasicProjectiveTextureMappingShader : IProjectiveTextureMappingShader
{
	float4 ComputeFinalColor(VS_OUTPUT IN)
	{
		PointLightShader pointLightShader;
		float4 OUT = pointLightShader.ComputeFinalColor(IN);

		IN.ProjectedMapCoordinates.xy /= IN.ProjectedMapCoordinates.w;
		float3 projectedColor = ProjectedMap.Sample(ProjectedMapSampler, IN.ProjectedMapCoordinates.xy).rgb;

		OUT.rgb *= projectedColor;

		return OUT;
	}
};

class NoReverseProjectiveTextureMappingShader : IProjectiveTextureMappingShader
{
	float4 ComputeFinalColor(VS_OUTPUT IN)
	{
		PointLightShader pointLightShader;
		float4 OUT = pointLightShader.ComputeFinalColor(IN);

		if (IN.ProjectedMapCoordinates.w >= 0.0f)
		{
			IN.ProjectedMapCoordinates.xy /= IN.ProjectedMapCoordinates.w;
			float3 projectedColor = ProjectedMap.Sample(ProjectedMapSampler, IN.ProjectedMapCoordinates.xy).rgb;

			OUT.rgb *= projectedColor;
		}

		return OUT;
	}
};

class DepthMapProjectiveTextureMappingShader : IProjectiveTextureMappingShader
{
	float4 ComputeFinalColor(VS_OUTPUT IN)
	{
		PointLightShader pointLightShader;
		float4 OUT = pointLightShader.ComputeFinalColor(IN);

		if (IN.ProjectedMapCoordinates.w >= 0.0f)
		{
			IN.ProjectedMapCoordinates.xyz /= IN.ProjectedMapCoordinates.w;
			float pixelDepth = IN.ProjectedMapCoordinates.z;
			float sampledDepth = DepthMap.Sample(DepthMapSampler, IN.ProjectedMapCoordinates.xy).x + DepthBias;

			float3 projectedColor = (pixelDepth > sampledDepth ? ColorWhite : ProjectedMap.Sample(ProjectedMapSampler, IN.ProjectedMapCoordinates.xy).rgb);
			OUT.rgb *= projectedColor;
		}

		return OUT;
	}
};

IProjectiveTextureMappingShader ProjectiveTextureMappingShader;

float4 main(VS_OUTPUT IN) : SV_TARGET
{
	return ProjectiveTextureMappingShader.ComputeFinalColor(IN);
}