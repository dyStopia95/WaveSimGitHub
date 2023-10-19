cbuffer CBufferPerFrame
{
	float3 CameraPosition;
	int MaxTessellationFactor;
	float MinTessellationDistance;
	float MaxTessellationDistance;
}

cbuffer CBufferPerObject
{
	float4x4 World;
}

struct VS_OUTPUT
{
	float4 ObjectPosition : SV_Position;
};

struct HS_CONSTANT_OUTPUT
{
	float EdgeFactors[3] : SV_TessFactor;
	float InsideFactor : SV_InsideTessFactor;
};

struct HS_OUTPUT
{
	float4 ObjectPosition : SV_Position;
};

HS_CONSTANT_OUTPUT constant_hull_shader(InputPatch<VS_OUTPUT, 3> patch, uint patchID : SV_PrimitiveID)
{
	HS_CONSTANT_OUTPUT OUT = (HS_CONSTANT_OUTPUT)0;

	// Caclulate the center of the patch
	float3 objectCenter = (patch[0].ObjectPosition.xyz + patch[1].ObjectPosition.xyz + patch[2].ObjectPosition.xyz) / 3.0f;
	float3 worldCenter = mul(float4(objectCenter, 1.0f), World).xyz;

	// Calculate uniform tessellation factor based on distance from the camera
	float tessellationFactor = max(min(MaxTessellationFactor, (MaxTessellationDistance - distance(worldCenter, CameraPosition)) / (MaxTessellationDistance - MinTessellationDistance) * MaxTessellationFactor), 1);

	[unroll]
	for (int i = 0; i < 3; i++)
	{
		OUT.EdgeFactors[i] = tessellationFactor;
	}

	OUT.InsideFactor = tessellationFactor;

	return OUT;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("constant_hull_shader")]
HS_OUTPUT main(InputPatch<VS_OUTPUT, 3> patch, uint controlPointID : SV_OutputControlPointID)
{
	HS_OUTPUT OUT = (HS_OUTPUT)0;

	OUT.ObjectPosition = patch[controlPointID].ObjectPosition;

	return OUT;
}