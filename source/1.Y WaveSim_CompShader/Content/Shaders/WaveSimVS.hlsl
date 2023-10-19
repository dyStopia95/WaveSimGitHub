cbuffer CBufferPerObject
{
    float4x4 WorldViewProjection;
}

struct VS_INPUT
{
    float2 NodePosition : XYPOS;
    uint Index : INDEX;
};

Texture1D<float2> HeightMap;

float4 main(VS_INPUT IN) : SV_Position
{
    float4 vertexPos = float4(0,0,0,1);
    vertexPos.x = IN.NodePosition.x;
    vertexPos.z = IN.NodePosition.y;
    vertexPos.y = HeightMap[IN.Index].x;
    vertexPos = mul(vertexPos, WorldViewProjection);
    
    return vertexPos;
}