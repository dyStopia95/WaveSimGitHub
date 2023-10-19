cbuffer CBufferPerObject
{
	float4x4 WorldViewProjection;
}

struct HS_CONSTANT_OUTPUT
{
	float EdgeFactors[3] : SV_TessFactor;
	float InsideFactor : SV_InsideTessFactor;
};

struct HS_OUTPUT
{
	float4 ObjectPosition : SV_Position;
};

struct DS_OUTPUT
{
	float4 Position : SV_Position;
};

[domain("tri")]
DS_OUTPUT main(HS_CONSTANT_OUTPUT IN, float3 uvw : SV_DomainLocation, const OutputPatch<HS_OUTPUT, 3> patch)
{
	DS_OUTPUT OUT = (DS_OUTPUT)0;

	float3 objectPosition = uvw.x * patch[0].ObjectPosition.xyz + uvw.y * patch[1].ObjectPosition.xyz + uvw.z * patch[2].ObjectPosition.xyz;

	OUT.Position = mul(float4(objectPosition, 1.0f), WorldViewProjection);

	return OUT;
}