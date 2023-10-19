static const unsigned int MaxBones = 60;

cbuffer CBufferPerObject
{
	float4x4 WorldViewProjection;
	float4x4 World;
}

cbuffer CBufferSkinning
{
	float4x4 BoneTransforms[MaxBones];
}

struct VS_INPUT
{
	float4 ObjectPosition: POSITION;
	float2 TextureCoordinates : TEXCOORD;
	float3 Normal : NORMAL;
	uint4 BoneIndices : BONEINDICES;
	float4 BoneWeights : BONEWEIGHTS;
};

struct VS_OUTPUT
{
	float4 Position: SV_Position;
	float2 TextureCoordinates : TEXCOORD;
	float3 Normal : NORMAL;
};

VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;

	float4x4 skinTransform = (float4x4)0;
	skinTransform += BoneTransforms[IN.BoneIndices.x] * IN.BoneWeights.x;
	skinTransform += BoneTransforms[IN.BoneIndices.y] * IN.BoneWeights.y;
	skinTransform += BoneTransforms[IN.BoneIndices.z] * IN.BoneWeights.z;
	skinTransform += BoneTransforms[IN.BoneIndices.w] * IN.BoneWeights.w;

	float4 position = mul(IN.ObjectPosition, skinTransform);
	OUT.Position = mul(position, WorldViewProjection);
	OUT.TextureCoordinates = IN.TextureCoordinates;

	float4 normal = mul(float4(IN.Normal, 0), skinTransform);
	OUT.Normal = normalize(mul(normal, World).xyz);

	return OUT;
}