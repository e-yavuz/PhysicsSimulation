inline float3 circlesCollision(float3 objectPosition, float objectRadius, float3 otherPosition, float otherRadius)
{
    float3 retval = objectPosition;
    float distance = fast_distance(objectPosition, otherPosition);

    if(distance < (objectRadius+otherRadius))
        retval = objectPosition - ((objectPosition-otherPosition)*(distance-(objectRadius+otherRadius)))/2;

    return retval;
}

#define local_size 32

__kernel void naive_Update(__global float* initPositions, __global float* finalPositions, __global float* radiusObjects, const uint N)
{
    __local float shared_pos[local_size*3];
    __local float shared_radius[local_size];
    ulong tx = get_global_id(0);
    ulong local_tx = get_local_id(0);
    float3 thisPosition;
    float thisRadius;
    if(tx < N)
    {
        thisPosition = (float3)(initPositions[3*tx], initPositions[(3*tx)+1], initPositions[(3*tx)+2]);
        thisRadius = radiusObjects[tx];
    }
    
    //Single Block
    for(ulong i = 0; i < N; i++)
    {
        float3 otherPosition = (float3)(initPositions[3*i],
                                        initPositions[(3*i)+1],
                                        initPositions[(3*i)+2]);
        float otherRadius = radiusObjects[i];
        thisPosition = circlesCollision(thisPosition, thisRadius, otherPosition, otherRadius);
    }

    if(tx < N)
    {
        finalPositions[3*tx] = thisPosition.x;
        finalPositions[(3*tx)+1] = thisPosition.y;
        finalPositions[(3*tx)+2] = thisPosition.z;
    }
}

// __kernel void naive_Update(__global float* initPositions, __global float* finalPositions, __global float* radiusObjects, const uint N)
// {
//     __local float shared_pos[local_size*3];
//     __local float shared_radius[local_size];
//     ulong tx = get_global_id(0);
//     ulong local_tx = get_local_id(0);
//     float3 thisPosition;
//     float thisRadius;
//     if(tx < N)
//     {
//         thisPosition = (float3)(initPositions[3*tx], initPositions[(3*tx)+1], initPositions[(3*tx)+2]);
//         thisRadius = radiusObjects[tx];
//     }
    
//     //Single Block
//     for(ulong offset = 0; offset < N; offset+=local_size)
//     {
//         ulong access_global = offset+local_tx;
//         if(access_global < N)
//         {
//             shared_pos[(3*local_tx)] = initPositions[(3*access_global)];
//             shared_pos[(3*local_tx)+1] = initPositions[(3*access_global)+1];
//             shared_pos[(3*local_tx)+2] = initPositions[(3*access_global)+2];
//             shared_pos[local_tx] = radiusObjects[access_global];
//         }

//         barrier(CLK_LOCAL_MEM_FENCE);
        
//         if(tx >= N)
//             continue;

//         for(ulong i = 0; i < 4; i++)
//         {
//             if(offset+i >= N)
//                 break;
//             float3 otherPosition = (float3)(shared_pos[3*i],
//                                             shared_pos[(3*i)+1],
//                                             shared_pos[(3*i)+2]);
//             float otherRadius = radiusObjects[i];
//             thisPosition = circlesCollision(thisPosition, thisRadius, otherPosition, otherRadius);
//         }
//     }

//     if(tx < N)
//     {
//         finalPositions[3*tx] = thisPosition.x;
//         finalPositions[(3*tx)+1] = thisPosition.y;
//         finalPositions[(3*tx)+2] = thisPosition.z;
//     }
// }


// __kernel void spatial_Hash_Update(__global float3* initObjects, __global float3* finalObjects, __constant uint* blockMetadataIndex, __constant uint2* metaData, const uint2 metaDataDim)
// {
//     ulong bx = get_group_id(0);

//     uint blockIndex = blockMetadataIndex[bx];

    
//     uint2 middleBlock = metaData[blockIndex];

//     float3 thisObject = initObjects[middleBlock.y+get_local_id(0)];
//     float2 thisPosition = (float2)(thisObject.x, thisObject.y);
//     float thisRadius = thisObject.z;

//     //NorthWest Block
//     if(blockIndex >= metaDataDim.y && blockIndex % metaDataDim.y != 0)
//     {
//         uint2 loopBlock = metaData[blockIndex-metaDataDim.y-1];
//         for(int i = 0; i < loopBlock.x; i++)
//         {
//             float3 otherObject = initObjects[loopBlock.y+i];
//             thisPosition = circlesCollision(thisPosition, thisRadius, (float2)(otherObject.x, otherObject.y), otherObject.z);
//         }
//     }

//     //North Block
//     if(blockIndex >= metaDataDim.y)
//     {
//         uint2 loopBlock = metaData[blockIndex-metaDataDim.y];
//         for(int i = 0; i < loopBlock.x; i++)
//         {
//             float3 otherObject = initObjects[loopBlock.y+i];
//             thisPosition = circlesCollision(thisPosition, thisRadius, (float2)(otherObject.x, otherObject.y), otherObject.z);
//         }
//     }

//     //NorthEast Block
//     if(blockIndex >= metaDataDim.y && blockIndex % metaDataDim.y != metaDataDim.y-1)
//     {
//         uint2 loopBlock = metaData[blockIndex-metaDataDim.y+1];
//         for(int i = 0; i < loopBlock.x; i++)
//         {
//             float3 otherObject = initObjects[loopBlock.y+i];
//             thisPosition = circlesCollision(thisPosition, thisRadius, (float2)(otherObject.x, otherObject.y), otherObject.z);
//         }
//     }

//     //West Block
//     if(blockIndex % metaDataDim.y != 0)
//     {
//         uint2 loopBlock = metaData[blockIndex-1];
//         for(int i = 0; i < loopBlock.x; i++)
//         {
//             float3 otherObject = initObjects[loopBlock.y+i];
//             thisPosition = circlesCollision(thisPosition, thisRadius, (float2)(otherObject.x, otherObject.y), otherObject.z);
//         }
//     }

//     //Middle Block
//     for(int i = 0; i < middleBlock.x; i++)
//     {
//         if( i == middleBlock.y+get_local_id(0) )
//             continue;
//         float3 otherObject = initObjects[middleBlock.y+i];
//         thisPosition = circlesCollision(thisPosition, thisRadius, (float2)(otherObject.x, otherObject.y), otherObject.z);
//     }

//     //East Block
//     if(blockIndex % metaDataDim.y != metaDataDim.y-1)
//     {
//         uint2 loopBlock = metaData[blockIndex+1];
//         for(int i = 0; i < loopBlock.x; i++)
//         {
//             float3 otherObject = initObjects[loopBlock.y+i];
//             thisPosition = circlesCollision(thisPosition, thisRadius, (float2)(otherObject.x, otherObject.y), otherObject.z);
//         }
//     }

//     //SouthWest Block
//     if(blockIndex < (metaDataDim.x-1)*metaDataDim.y && blockIndex % metaDataDim.y != 0)
//     {
//         uint2 loopBlock = metaData[blockIndex+metaDataDim.y-1];
//         for(int i = 0; i < loopBlock.x; i++)
//         {
//             float3 otherObject = initObjects[loopBlock.y+i];
//             thisPosition = circlesCollision(thisPosition, thisRadius, (float2)(otherObject.x, otherObject.y), otherObject.z);
//         }
//     }

//     //South Block
//     if(blockIndex < (metaDataDim.y-1)*metaDataDim.y)
//     {
//         uint2 loopBlock = metaData[blockIndex+metaDataDim.y];
//         for(int i = 0; i < loopBlock.x; i++)
//         {
//             float3 otherObject = initObjects[loopBlock.y+i];
//             thisPosition = circlesCollision(thisPosition, thisRadius, (float2)(otherObject.x, otherObject.y), otherObject.z);
//         }
//     }

//     //SouthEast Block
//     if(blockIndex < (metaDataDim.x-1)*metaDataDim.y && blockIndex % metaDataDim.y != metaDataDim.y-1)
//     {
//         uint2 loopBlock = metaData[blockIndex+metaDataDim.y+1];
//         for(int i = 0; i < loopBlock.x; i++)
//         {
//             float3 otherObject = initObjects[loopBlock.y+i];
//             thisPosition = circlesCollision(thisPosition, thisRadius, (float2)(otherObject.x, otherObject.y), otherObject.z);
//         }
//     }

    
//     finalObjects[middleBlock.y+get_local_id(0)].x = thisPosition.x;
//     finalObjects[middleBlock.y+get_local_id(0)].y = thisPosition.y;
// }
