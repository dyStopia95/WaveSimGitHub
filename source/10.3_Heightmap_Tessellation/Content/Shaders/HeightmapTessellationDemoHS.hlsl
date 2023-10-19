cbuffer CBufferPerFrame
{
	float4 TessellationEdgeFactors;
	float2 TessellationInsideFactors;
}

struct VS_OUTPUT
{
	float4 ObjectPosition : SV_Position;
	float2 TextureCoordinates: TEXCOORD;
};

struct HS_CONSTANT_OUTPUT
{
	float EdgeFactors[4] : SV_TessFactor;
	float InsideFactors[2] : SV_InsideTessFactor;
};

struct HS_OUTPUT
{
	float4 ObjectPosition : SV_Position;
	float2 TextureCoordinates: TEXCOORD;
};

HS_CONSTANT_OUTPUT constant_hull_shader(InputPatch<VS_OUTPUT, 4> patch, uint patchID : SV_PrimitiveID)
{
	HS_CONSTANT_OUTPUT OUT = (HS_CONSTANT_OUTPUT)0;

	OUT.EdgeFactors[0] = TessellationEdgeFactors.x;
	OUT.EdgeFactors[1] = TessellationEdgeFactors.y;
	OUT.EdgeFactors[2] = TessellationEdgeFactors.z;
	OUT.EdgeFactors[3] = TessellationEdgeFactors.w;

	OUT.InsideFactors[0] = TessellationInsideFactors.x;
	OUT.InsideFactors[1] = TessellationInsideFactors.y;

	return OUT;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("constant_hull_shader")]
HS_OUTPUT main(InputPatch<VS_OUTPUT, 4> patch, uint controlPointID : SV_OutputControlPointID)
{
	HS_OUTPUT OUT = (HS_OUTPUT)0;

	OUT.ObjectPosition = patch[controlPointID].ObjectPosition;
	OUT.TextureCoordinates = patch[controlPointID].TextureCoordinates;

	return OUT;
}