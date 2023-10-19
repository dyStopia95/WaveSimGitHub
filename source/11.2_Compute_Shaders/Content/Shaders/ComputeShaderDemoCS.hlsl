RWTexture2D<float4> OutputTexture;

cbuffer CBufferPerFrame
{
	float2 TextureSize;
	float BlueColor;
};

[numthreads(32, 32, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
	OutputTexture[threadID.xy] = float4((threadID.xy / TextureSize), BlueColor, 1);
}