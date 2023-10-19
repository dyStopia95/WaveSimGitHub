RWTexture1D<float2> NodeZVel;

cbuffer CBufferPerFrame
{
    float C2;
    float dampingFactor;
    float k;
    float deltaT;
    float spacing2;
    int rows;
    int columns;
    int nodeCount;
};

//Texture1D<float2> VertexXY;

[numthreads(1, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    //OutputTexture[threadID.xy] = float4((threadID.xy / TextureSize), BlueColor, 1);
    int up = 0;
    int down = 0;
    int left = 0;
    int right = 0;
    
    up = threadID.x - columns;
    if(up < 0)
        up = threadID.x;
    
    down = threadID.x + columns;
    if (down > (nodeCount - 1))
        down = threadID.x;
    
    left = threadID.x - 1;
    if ((left % columns) == 0)
        left = threadID.x;
    
    right = threadID.x + 1;
    if(right % columns == 0)
        right = threadID.x;
    
    float curvature = (NodeZVel[up].x + NodeZVel[down].x + NodeZVel[left].x + NodeZVel[right].x - (4 * NodeZVel[threadID.x].x)) / spacing2;
    
    float acceleration = ((C2 * curvature) - (dampingFactor * NodeZVel[threadID.x].y) - (k * NodeZVel[threadID.x].x));
    float velocity = NodeZVel[threadID.x].y + acceleration * deltaT;
    float displacement = NodeZVel[threadID.x].x + velocity * deltaT;
    
    NodeZVel[threadID.x] = float2(displacement, velocity);
}