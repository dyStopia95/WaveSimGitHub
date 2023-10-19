struct VS_INPUT
{
	float4 ObjectPosition : POSITION;
};

struct VS_OUTPUT
{
	float4 ObjectPosition : SV_Position;
};

VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;

	OUT.ObjectPosition = IN.ObjectPosition;

	return OUT;
}