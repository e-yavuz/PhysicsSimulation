#include <opencl-c-base.h>
inline float2 circlesCollision(float2 objectPosition, float objectRadius, float2 otherPosition, float otherRadius)
{
    float2 retval = objectPosition;
    float distance = fast_distance(objectPosition, otherPosition);

    if(distance < (objectRadius+otherRadius))
        retval = objectPosition - ((objectPosition-otherPosition)*(distance-(objectRadius+otherRadius)))/2;

    return retval;
}
//Have n blocks queued each with preset size, if it can't all fit into one thread block then assume another thread block is handling the rest
//How does one get the neighbors of a given thread block?

//Perhaps have a very large 1D array, with each internal section acting like a std::vector, doubling in capacity upon reaching limit?
//Would then be a tradeoff between memory and performance :/

__kernel void update(__global float3* initObjects, __global float3* finalObjects, __constant uint* blockMetadataIndex, __constant uint2* metaData, const uint2 metaDataDim)
{
    ulong bx = get_group_id(0);

    uint blockIndex = blockMetadataIndex[bx];

    
    uint2 middleBlock = metaData[blockIndex];

    float3 thisObject = initObjects[middleBlock.y+get_local_id(0)];
    float2 thisPosition = (float2)(thisObject.x, thisObject.y);
    float thisRadius = thisObject.z;

    //NorthWest Block
    if(blockIndex >= metaDataDim.y && blockIndex % metaDataDim.y != 0)
    {
        uint2 loopBlock = metaData[blockIndex-metaDataDim.y-1];
        for(int i = 0; i < loopBlock.x; i++)
        {
            float3 otherObject = initObjects[loopBlock.y+i];
            thisPosition = circlesCollision(thisPosition, thisRadius, (float2)(otherObject.x, otherObject.y), otherObject.z);
        }
    }

    //North Block
    if(blockIndex >= metaDataDim.y)
    {
        uint2 loopBlock = metaData[blockIndex-metaDataDim.y];
        for(int i = 0; i < loopBlock.x; i++)
        {
            float3 otherObject = initObjects[loopBlock.y+i];
            thisPosition = circlesCollision(thisPosition, thisRadius, (float2)(otherObject.x, otherObject.y), otherObject.z);
        }
    }

    //NorthEast Block
    if(blockIndex >= metaDataDim.y && blockIndex % metaDataDim.y != metaDataDim.y-1)
    {
        uint2 loopBlock = metaData[blockIndex-metaDataDim.y+1];
        for(int i = 0; i < loopBlock.x; i++)
        {
            float3 otherObject = initObjects[loopBlock.y+i];
            thisPosition = circlesCollision(thisPosition, thisRadius, (float2)(otherObject.x, otherObject.y), otherObject.z);
        }
    }

    //West Block
    if(blockIndex % metaDataDim.y != 0)
    {
        uint2 loopBlock = metaData[blockIndex-1];
        for(int i = 0; i < loopBlock.x; i++)
        {
            float3 otherObject = initObjects[loopBlock.y+i];
            thisPosition = circlesCollision(thisPosition, thisRadius, (float2)(otherObject.x, otherObject.y), otherObject.z);
        }
    }

    //Middle Block
    for(int i = 0; i < middleBlock.x; i++)
    {
        if( i == middleBlock.y+get_local_id(0) )
            continue;
        float3 otherObject = initObjects[middleBlock.y+i];
        thisPosition = circlesCollision(thisPosition, thisRadius, (float2)(otherObject.x, otherObject.y), otherObject.z);
    }

    //East Block
    if(blockIndex % metaDataDim.y != metaDataDim.y-1)
    {
        uint2 loopBlock = metaData[blockIndex+1];
        for(int i = 0; i < loopBlock.x; i++)
        {
            float3 otherObject = initObjects[loopBlock.y+i];
            thisPosition = circlesCollision(thisPosition, thisRadius, (float2)(otherObject.x, otherObject.y), otherObject.z);
        }
    }

    //SouthWest Block
    if(blockIndex < (metaDataDim.x-1)*metaDataDim.y && blockIndex % metaDataDim.y != 0)
    {
        uint2 loopBlock = metaData[blockIndex+metaDataDim.y-1];
        for(int i = 0; i < loopBlock.x; i++)
        {
            float3 otherObject = initObjects[loopBlock.y+i];
            thisPosition = circlesCollision(thisPosition, thisRadius, (float2)(otherObject.x, otherObject.y), otherObject.z);
        }
    }

    //South Block
    if(blockIndex < (metaDataDim.y-1)*metaDataDim.y)
    {
        uint2 loopBlock = metaData[blockIndex+metaDataDim.y];
        for(int i = 0; i < loopBlock.x; i++)
        {
            float3 otherObject = initObjects[loopBlock.y+i];
            thisPosition = circlesCollision(thisPosition, thisRadius, (float2)(otherObject.x, otherObject.y), otherObject.z);
        }
    }

    //SouthEast Block
    if(blockIndex < (metaDataDim.x-1)*metaDataDim.y && blockIndex % metaDataDim.y != metaDataDim.y-1)
    {
        uint2 loopBlock = metaData[blockIndex+metaDataDim.y+1];
        for(int i = 0; i < loopBlock.x; i++)
        {
            float3 otherObject = initObjects[loopBlock.y+i];
            thisPosition = circlesCollision(thisPosition, thisRadius, (float2)(otherObject.x, otherObject.y), otherObject.z);
        }
    }

    
    finalObjects[middleBlock.y+get_local_id(0)].x = thisPosition.x;
    finalObjects[middleBlock.y+get_local_id(0)].y = thisPosition.y;
}
