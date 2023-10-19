cbuffer CBufferPerFrame
{
	float3 TessellationEdgeFactors;
	float TessellationInsideFactor;
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

HS_CONSTANT_OUTPUT constant_hull_shader(InputPatch<VS_OUTPUT, 3> patch)
{
	HS_CONSTANT_OUTPUT OUT = (HS_CONSTANT_OUTPUT)0;

	OUT.EdgeFactors[0] = TessellationEdgeFactors.x;
	OUT.EdgeFactors[1] = TessellationEdgeFactors.y;
	OUT.EdgeFactors[2] = TessellationEdgeFactors.z;

	OUT.InsideFactor = TessellationInsideFactor;

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