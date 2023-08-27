inline float3 circlesCollision(float3 objectPosition, float objectRadius, float3 otherPosition, float otherRadius)
{
    float3 retval = objectPosition;
    float distance = fast_distance(objectPosition, otherPosition);

    if(distance < (objectRadius+otherRadius))
        retval = objectPosition - ((objectPosition-otherPosition)*(distance-(objectRadius+otherRadius)))/2;

    return retval;
}

__kernel void spatial_Hash_Update(__global float* initPositions, __global float* finalPositions, __global float* radiusObjects, __global uint* spatialIndicies, __global uint2* threadBlockToSpatialHashMetaData, __global uint2* spatialHashGridMetaData, const uint spatialHashGridWidth)
{
    ulong bx = get_group_id(0);
    ulong local_tx = get_local_id(0);

    // x: offset in block
    // y: block Index
    uint2 thisThreadBlockToSpatialHashMetaData = threadBlockToSpatialHashMetaData[bx];
    uint indexOfBlock = thisThreadBlockToSpatialHashMetaData[1];
    
    // x: starting index of block
    // y: size of block
    uint2 middleBlock = spatialHashGridMetaData[indexOfBlock];

    if(thisThreadBlockToSpatialHashMetaData[0]+local_tx >= middleBlock[1])
        return;

    uint thisIndex = spatialIndicies[middleBlock[0]+thisThreadBlockToSpatialHashMetaData[0]+local_tx];

    float3 thisPosition = (float3)(initPositions[3*thisIndex], initPositions[(3*thisIndex)+1], initPositions[(3*thisIndex)+2]);
    float thisRadius = radiusObjects[thisIndex];

    //NorthWest Block
    if(indexOfBlock >= spatialHashGridWidth && indexOfBlock % spatialHashGridWidth != 0)
    {
        uint2 loopBlock = spatialHashGridMetaData[indexOfBlock-spatialHashGridWidth-1];
        for(int i = loopBlock[0]; i < loopBlock[1]+loopBlock[0]; i++)
        {
            uint otherIndex = spatialIndicies[i];
            float3 otherPosition = (float3)(initPositions[3*otherIndex],
                                            initPositions[(3*otherIndex)+1],
                                            initPositions[(3*otherIndex)+2]);
            float otherRadius = radiusObjects[otherIndex];
            thisPosition = circlesCollision(thisPosition, thisRadius, otherPosition, otherRadius);
        }
    }

    //North Block
    if(indexOfBlock >= spatialHashGridWidth)
    {
        uint2 loopBlock = spatialHashGridMetaData[indexOfBlock-spatialHashGridWidth];
        for(int i = loopBlock[0]; i < loopBlock[1]+loopBlock[0]; i++)
        {
            uint otherIndex = spatialIndicies[i];
            float3 otherPosition = (float3)(initPositions[3*otherIndex],
                                            initPositions[(3*otherIndex)+1],
                                            initPositions[(3*otherIndex)+2]);
            float otherRadius = radiusObjects[otherIndex];
            thisPosition = circlesCollision(thisPosition, thisRadius, otherPosition, otherRadius);
        }
    }

    //NorthEast Block
    if(indexOfBlock >= spatialHashGridWidth && indexOfBlock % spatialHashGridWidth != spatialHashGridWidth-1)
    {
        uint2 loopBlock = spatialHashGridMetaData[indexOfBlock-spatialHashGridWidth+1];
        for(int i = loopBlock[0]; i < loopBlock[1]+loopBlock[0]; i++)
        {
            uint otherIndex = spatialIndicies[i];
            float3 otherPosition = (float3)(initPositions[3*otherIndex],
                                            initPositions[(3*otherIndex)+1],
                                            initPositions[(3*otherIndex)+2]);
            float otherRadius = radiusObjects[otherIndex];
            thisPosition = circlesCollision(thisPosition, thisRadius, otherPosition, otherRadius);
        }
    }

    //West Block
    if(indexOfBlock % spatialHashGridWidth != 0)
    {
        uint2 loopBlock = spatialHashGridMetaData[indexOfBlock-1];
        for(int i = loopBlock[0]; i < loopBlock[1]+loopBlock[0]; i++)
        {
            uint otherIndex = spatialIndicies[i];
            float3 otherPosition = (float3)(initPositions[3*otherIndex],
                                            initPositions[(3*otherIndex)+1],
                                            initPositions[(3*otherIndex)+2]);
            float otherRadius = radiusObjects[otherIndex];
            thisPosition = circlesCollision(thisPosition, thisRadius, otherPosition, otherRadius);
        }
    }

    // //Middle Block
    for(int i = middleBlock[0]; i < middleBlock[0]+middleBlock[1]; i++)
    {
        uint otherIndex = spatialIndicies[i];
        float3 otherPosition = (float3)(initPositions[3*otherIndex],
                                            initPositions[(3*otherIndex)+1],
                                            initPositions[(3*otherIndex)+2]);
        float otherRadius = radiusObjects[otherIndex];
        thisPosition = circlesCollision(thisPosition, thisRadius, otherPosition, otherRadius);
    }

    //East Block
    if(indexOfBlock % spatialHashGridWidth != spatialHashGridWidth-1)
    {
        uint2 loopBlock = spatialHashGridMetaData[indexOfBlock+1];
        for(int i = loopBlock[0]; i < loopBlock[1]+loopBlock[0]; i++)
        {
            uint otherIndex = spatialIndicies[i];
            float3 otherPosition = (float3)(initPositions[3*otherIndex],
                                            initPositions[(3*otherIndex)+1],
                                            initPositions[(3*otherIndex)+2]);
            float otherRadius = radiusObjects[otherIndex];
            thisPosition = circlesCollision(thisPosition, thisRadius, otherPosition, otherRadius);
        }
    }

    //SouthWest Block
    if(indexOfBlock < (spatialHashGridWidth-1)*spatialHashGridWidth && indexOfBlock % spatialHashGridWidth != 0)
    {
        uint2 loopBlock = spatialHashGridMetaData[indexOfBlock+spatialHashGridWidth-1];
        for(int i = loopBlock[0]; i < loopBlock[1]+loopBlock[0]; i++)
        {
            uint otherIndex = spatialIndicies[i];
            float3 otherPosition = (float3)(initPositions[3*otherIndex],
                                            initPositions[(3*otherIndex)+1],
                                            initPositions[(3*otherIndex)+2]);
            float otherRadius = radiusObjects[otherIndex];
            thisPosition = circlesCollision(thisPosition, thisRadius, otherPosition, otherRadius);
        }
    }

    //South Block
    if(indexOfBlock < (spatialHashGridWidth-1)*spatialHashGridWidth)
    {
        uint2 loopBlock = spatialHashGridMetaData[indexOfBlock+spatialHashGridWidth];
        for(int i = loopBlock[0]; i < loopBlock[1]+loopBlock[0]; i++)
        {
            uint otherIndex = spatialIndicies[i];
            float3 otherPosition = (float3)(initPositions[3*otherIndex],
                                            initPositions[(3*otherIndex)+1],
                                            initPositions[(3*otherIndex)+2]);
            float otherRadius = radiusObjects[otherIndex];
            thisPosition = circlesCollision(thisPosition, thisRadius, otherPosition, otherRadius);
        }
    }

    //SouthEast Block
    if(indexOfBlock < (spatialHashGridWidth-1)*spatialHashGridWidth && indexOfBlock % spatialHashGridWidth != spatialHashGridWidth-1)
    {
        uint2 loopBlock = spatialHashGridMetaData[indexOfBlock+spatialHashGridWidth+1];
        for(int i = loopBlock[0]; i < loopBlock[1]+loopBlock[0]; i++)
        {
            uint otherIndex = spatialIndicies[i];
            float3 otherPosition = (float3)(initPositions[3*otherIndex],
                                            initPositions[(3*otherIndex)+1],
                                            initPositions[(3*otherIndex)+2]);
            float otherRadius = radiusObjects[otherIndex];
            thisPosition = circlesCollision(thisPosition, thisRadius, otherPosition, otherRadius);
        }
    }
    
    finalPositions[3*thisIndex] = thisPosition.x;
    finalPositions[(3*thisIndex)+1] = thisPosition.y;
    finalPositions[(3*thisIndex)+2] = thisPosition.z;
}

__kernel void naive_Update(__global float* initPositions, __global float* finalPositions, __global float* radiusObjects, const uint N)
{
    ulong tx = get_global_id(0);
    ulong local_tx = get_local_id(0);
    float3 thisPosition;
    float thisRadius;

    if(tx >= N)
        return;
    
    thisPosition = (float3)(initPositions[3*tx], initPositions[(3*tx)+1], initPositions[(3*tx)+2]);
    thisRadius = radiusObjects[tx];
    
    //Single Block
    for(ulong i = 0; i < N; i++)
    {
        float3 otherPosition = (float3)(initPositions[3*i],
                                        initPositions[(3*i)+1],
                                        initPositions[(3*i)+2]);
        float otherRadius = radiusObjects[i];
        thisPosition = circlesCollision(thisPosition, thisRadius, otherPosition, otherRadius);
    }

    finalPositions[3*tx] = thisPosition.x;
    finalPositions[(3*tx)+1] = thisPosition.y;
    finalPositions[(3*tx)+2] = thisPosition.z;
}