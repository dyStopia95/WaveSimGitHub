static const float2 QuadStripUVs[4] = { float2(0.0f, 1.0f), // v0, lower-left
										float2(0.0f, 0.0f), // v1, upper-left
										float2(1.0f, 1.0f), // v2, lower-right
										float2(1.0f, 0.0f)  // v3, upper-right										
};

cbuffer CBufferPerFrame
{
	float4x4 ViewProjectionMatrix;
	float3 CameraPosition;
	float3 CameraUp;
}

struct VS_OUTPUT
{
	float4 Position : SV_Position;
	float2 Size : SIZE;
};

struct GS_OUTPUT
{
	float4 Position : SV_Position;
	float2 TextureCoordinates : TEXCOORD;
};

[maxvertexcount(4)]
void main(point VS_OUTPUT IN[1], inout TriangleStream<GS_OUTPUT> triStream)
{
	GS_OUTPUT OUT = (GS_OUTPUT)0;

	float2 halfSize = IN[0].Size / 2.0f;
	float3 direction = CameraPosition - IN[0].Position.xyz;
	float3 right = cross(normalize(direction), CameraUp);

	float3 offsetX = halfSize.x * right;
	float3 offsetY = halfSize.y * CameraUp;

	float4 vertices[4];
	vertices[0] = float4(IN[0].Position.xyz + offsetX - offsetY, 1.0f); // lower-left
	vertices[1] = float4(IN[0].Position.xyz + offsetX + offsetY, 1.0f); // upper-left	
	vertices[2] = float4(IN[0].Position.xyz - offsetX - offsetY, 1.0f); // lower-right
	vertices[3] = float4(IN[0].Position.xyz - offsetX + offsetY, 1.0f); // upper-right

	[unroll]
	for (int i = 0; i < 4; i++)
	{
		OUT.Position = mul(vertices[i], ViewProjectionMatrix);
		OUT.TextureCoordinates = QuadStripUVs[i];
		triStream.Append(OUT);
	}
}