struct VS_INPUT
{
	float4 Position : POSITION;
	float2 Size : SIZE;
};

struct VS_OUTPUT
{
	float4 Position : SV_Position;
	float2 Size : SIZE;
};

VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;

	OUT.Position = IN.Position;
	OUT.Size = IN.Size;

	return OUT;
}