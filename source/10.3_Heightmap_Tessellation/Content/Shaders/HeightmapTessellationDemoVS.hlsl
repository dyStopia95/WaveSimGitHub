cbuffer CBufferPerObject
{
	float4x4 TextureMatrix;
}

struct VS_INPUT
{
	float4 ObjectPosition : POSITION;
	float2 TextureCoordinates: TEXCOORD;
};

struct VS_OUTPUT
{
	float4 ObjectPosition : SV_Position;
	float2 TextureCoordinates: TEXCOORD;
};

VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;

	OUT.ObjectPosition = IN.ObjectPosition;
	OUT.TextureCoordinates = mul(float4(IN.TextureCoordinates, 0, 1), TextureMatrix).xy;

	return OUT;
}