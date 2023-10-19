static const float4 ColorWheat = { 0.961f, 0.871f, 0.702f, 1.0f };

struct DS_OUTPUT
{
	float4 Position : SV_Position;
};

float4 main(DS_OUTPUT IN) : SV_TARGET
{
	return ColorWheat;
}