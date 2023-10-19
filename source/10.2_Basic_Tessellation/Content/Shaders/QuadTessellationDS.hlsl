cbuffer CBufferPerObject
{
	float4x4 WorldViewProjection;
}

struct HS_CONSTANT_OUTPUT
{
	float EdgeFactors[4] : SV_TessFactor;
	float InsideFactors[2] : SV_InsideTessFactor;
};

struct HS_OUTPUT
{
	float4 ObjectPosition : SV_Position;
};

struct DS_OUTPUT
{
	float4 Position : SV_Position;
};

[domain("quad")]
DS_OUTPUT main(HS_CONSTANT_OUTPUT IN, float2 uv : SV_DomainLocation, const OutputPatch<HS_OUTPUT, 4> patch)
{
	DS_OUTPUT OUT;

	float4 v0 = lerp(patch[0].ObjectPosition, patch[1].ObjectPosition, uv.x);
	float4 v1 = lerp(patch[2].ObjectPosition, patch[3].ObjectPosition, uv.x);
	float4 objectPosition = lerp(v0, v1, uv.y);

	OUT.Position = mul(float4(objectPosition.xyz, 1.0f), WorldViewProjection);

	return OUT;
}