static const float ZeroCorrection = 0.5f / 255.0f;

interface IDistortionShader
{
	float4 ComputeFinalColor(float2 uv);
};

cbuffer CBufferPerObject
{
	float DisplacementScale;
}

IDistortionShader DistortionShader;
Texture2D ColorMap : register(t0);
Texture2D DistortionMap : register(t1);
SamplerState TextureSampler;

struct VS_OUTPUT
{
	float4 Position : SV_Position;
	float2 TextureCoordinates : TEXCOORD;
};

class CutoutDistortionShader : IDistortionShader
{
	float4 ComputeFinalColor(float2 uv)
	{
		float2 displacement = DistortionMap.Sample(TextureSampler, uv).xy;
		return float4(displacement.xy, 0, 1);
	}
};

class CompositeDistortionShader : IDistortionShader
{
	float4 ComputeFinalColor(float2 uv)
	{
		float2 displacement = DistortionMap.Sample(TextureSampler, uv).xy;
		if (displacement.x != 0 || displacement.y != 0)
		{
			displacement -= 0.5f + ZeroCorrection;
			uv += DisplacementScale * displacement;
		}

		return ColorMap.Sample(TextureSampler, uv);
	}
};

class NoDistortionShader : IDistortionShader
{
	float4 ComputeFinalColor(float2 uv)
	{
		return ColorMap.Sample(TextureSampler, uv);
	}
};

float4 main(VS_OUTPUT IN) : SV_TARGET
{
	return DistortionShader.ComputeFinalColor(IN.TextureCoordinates);
}