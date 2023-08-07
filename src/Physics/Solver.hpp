#ifndef SOLVER_H
#define SOLVER_H

#include "Entities/Circle.hpp"
#include "Entities/VerletObject.hpp"
#include <memory>
#include <unordered_map>
#include <utility>

template<typename T>
class VertexContainer;

#define GRAVITY vertex(0, -0.001f, 0)
#define SPATIAL_HASH_GRANULARITY float(0.05f)
#define SPATIAL_HASH_TABLE_DIM_SIZE (int32_t(2.0f/SPATIAL_HASH_GRANULARITY))
#define SPATIAL_HASH_LOOKUP_TABLE_SIZE int32_t(SPATIAL_HASH_TABLE_DIM_SIZE*SPATIAL_HASH_TABLE_DIM_SIZE*2)

class Solver{
public:
    void Update(int subSteps)
    {
        for(int i = 0; i < subSteps; i++)
        {
            applyUpdate(1.0f/subSteps);
            applyConstraints();
            applyCollision();
            applyGravity();
            applyConstraints();
            applyDraw();
        }
    }

    void addCircle(float x, float y, float z, float radius, int vCount)
    {
        vertex currentPosition = objectsCurrentPosition.emplace_back(x,y,z);
        objectsOldPosition.emplace_back(x,y,z);
        objectsAcceleration.emplace_back(0,0,0);
        circlesRadii.push_back(radius);
        uint32_t start_ind = vertices->size();
        Circle::buildCircle(currentPosition, radius, vCount, vertices);
        uint32_t end_ind = vertices->size();
        objectsVertexIndicies.emplace_back(start_ind, end_ind);
    }

    std::vector<vertex> objectsCurrentPosition;
    std::vector<vertex> objectsOldPosition;
    std::vector<vertex> objectsAcceleration;
    std::vector<float> circlesRadii;
    std::vector<uint32_t> spatialHashIndicies;
    // MxM spatial Hash Lookup Table where [0] reperesents start in spatialHashIndicies 
    // and [1] represents the size
    uint32_t spatialHashLookupTable_A[SPATIAL_HASH_LOOKUP_TABLE_SIZE];
    uint32_t spatialHashLookupTable_B[SPATIAL_HASH_LOOKUP_TABLE_SIZE];
    uint32_t* spatialHashLookupTable_ptr;
    std::vector<std::pair<uint32_t, uint32_t>> objectsVertexIndicies;
    VertexContainer<float>* vertices;
private:
    void applyConstraints()
    {
        vertex center(0,0,0);
        for(int i = 0; i < objectsCurrentPosition.size(); i++)
        {
            vertex& current = objectsCurrentPosition[i];
            float boundary = 1.0f-circlesRadii[i];

            if(current.x > boundary)
                current.x = boundary-(current.x-boundary);
            else if(current.x < -boundary)
                current.x = (-boundary)-(current.x+boundary);

            if(current.y > boundary)
                current.y = boundary-(current.y-boundary);
            else if(current.y < -boundary)
                current.y = (-boundary)-(current.y+boundary);
            
            if(current.z > boundary)
                current.z = boundary-(current.z-boundary);
            else if(current.z < -boundary)
                current.z = (-boundary)-(current.z+boundary);
        }
    }

    void applyCollision()
    {
        // Spatial Hash CPU solution
        applySpatialHash();
        for(uint32_t y = 0; y < SPATIAL_HASH_TABLE_DIM_SIZE; y++)
        {
            for(uint32_t x = 0; x < SPATIAL_HASH_TABLE_DIM_SIZE; x++)
            {
                uint32_t spatialIndex = (y*SPATIAL_HASH_TABLE_DIM_SIZE*2) + (x*2);
                for(int32_t i = spatialHashLookupTable_ptr[spatialIndex]; i < spatialHashLookupTable_ptr[spatialIndex]+spatialHashLookupTable_ptr[spatialIndex+1]; i++)
                {
                    int32_t objectIndex = spatialHashIndicies[i];
                    checkSpatialHash(x, y, objectIndex);

                    checkSpatialHash(x, y+1, objectIndex);
                    checkSpatialHash(x, y-1, objectIndex);

                    checkSpatialHash(x+1, y, objectIndex);
                    checkSpatialHash(x-1, y, objectIndex);

                    checkSpatialHash(x+1, y+1, objectIndex);
                    checkSpatialHash(x-1, y+1, objectIndex);

                    checkSpatialHash(x+1, y-1, objectIndex);
                    checkSpatialHash(x-1, y-1, objectIndex);
                }
            }
        }

        // O(n^2) solution
        // for(uint32_t i = 0; i < objectsCurrentPosition.size(); i++)
        //     for(uint32_t j = 0; j < objectsCurrentPosition.size(); j++)
        //         checkCollision(i, j);
    }

    void checkSpatialHash(uint32_t x, uint32_t y, uint32_t objectIndex)
    {
        if(x < 0 || x >= SPATIAL_HASH_TABLE_DIM_SIZE) return;
        if(y < 0 || y >= SPATIAL_HASH_TABLE_DIM_SIZE) return;

        uint32_t spatialIndex = (y*SPATIAL_HASH_TABLE_DIM_SIZE*2) + (x*2);

        for(int32_t i = spatialHashLookupTable_ptr[spatialIndex]; i < spatialHashLookupTable_ptr[spatialIndex]+spatialHashLookupTable_ptr[spatialIndex+1]; i++)
            checkCollision(objectIndex, spatialHashIndicies[i]);
    }

    inline void checkCollision(uint32_t objectIndex, uint32_t otherIndex)
    {
        if(objectIndex == otherIndex) return;
        float objectRadius = circlesRadii[objectIndex];
        float otherRadius = circlesRadii[otherIndex];

        vertex objectPosition = objectsCurrentPosition[objectIndex];
        vertex otherPosition = objectsCurrentPosition[otherIndex];

        float distance = objectPosition.distance(otherPosition);

        if(distance < (objectRadius+otherRadius))
            objectsCurrentPosition[objectIndex] = objectPosition - (objectPosition-otherPosition)*(distance-(objectRadius+otherRadius));
    }

    void applySpatialHash()
    {
        uint32_t* REPLACEMENT_spatialHashLookupTable_ptr;
        if(spatialHashLookupTable_ptr == spatialHashLookupTable_A)
            REPLACEMENT_spatialHashLookupTable_ptr = &spatialHashLookupTable_B[0];
        else
            REPLACEMENT_spatialHashLookupTable_ptr = &spatialHashLookupTable_A[0];
        std::fill_n(REPLACEMENT_spatialHashLookupTable_ptr, SPATIAL_HASH_LOOKUP_TABLE_SIZE, 0);
        // Remake spatial Hash Indicies Table, but with extra +1 at end 
        // to allow for future addition of an element
        std::vector<uint32_t> new_SpatialHashIndicies(objectsCurrentPosition.size()+1);



        // Iterate through objectCurrentPositions, updating the new LookupTable sizes
        for(vertex& objectPosition: objectsCurrentPosition)
        {
            int32_t x = int32_t((objectPosition.x+1.0f)/SPATIAL_HASH_GRANULARITY);
            int32_t y = int32_t((objectPosition.y+1.0f)/SPATIAL_HASH_GRANULARITY);
            REPLACEMENT_spatialHashLookupTable_ptr[(y*SPATIAL_HASH_TABLE_DIM_SIZE*2)+(x*2)]++;
            REPLACEMENT_spatialHashLookupTable_ptr[(y*SPATIAL_HASH_TABLE_DIM_SIZE*2)+(x*2)+1]++;
        }

        // Iterate through LookupTable, setting serially each indicies' starting position
        // kind of like the fibonacci sequence but with an extra, random number for each element
        for(uint32_t i = 2; i < SPATIAL_HASH_LOOKUP_TABLE_SIZE; i+=2)
            REPLACEMENT_spatialHashLookupTable_ptr[i] += REPLACEMENT_spatialHashLookupTable_ptr[i-2];

        for(int32_t i = 0; i < objectsCurrentPosition.size(); i++)
        {
            vertex& objectPosition = objectsCurrentPosition[i];
            int32_t x = int32_t((objectPosition.x+1.0f)/SPATIAL_HASH_GRANULARITY);
            int32_t y = int32_t((objectPosition.y+1.0f)/SPATIAL_HASH_GRANULARITY);
            int32_t insert_position = --REPLACEMENT_spatialHashLookupTable_ptr[(y*SPATIAL_HASH_TABLE_DIM_SIZE*2)+(x*2)];
            new_SpatialHashIndicies[insert_position] = i;
        }

        spatialHashIndicies = new_SpatialHashIndicies;
        spatialHashLookupTable_ptr = REPLACEMENT_spatialHashLookupTable_ptr;

    }

    void applyDraw()
    {
        for(int i = 0; i < objectsCurrentPosition.size(); i++)
            VerletObject::UpdateVerticies(objectsCurrentPosition[i]-objectsOldPosition[i], 
            objectsVertexIndicies[i],
            vertices->data);
    }

    void applyGravity()
    {
        for(int i = 0; i < objectsAcceleration.size(); i++)
            objectsAcceleration[i] = GRAVITY;
    }
    void applyUpdate(float dt)
    {
        for(int i = 0; i < objectsCurrentPosition.size(); i++)
            VerletObject::Update(objectsCurrentPosition[i], 
            objectsOldPosition[i], 
            objectsAcceleration[i], 
            dt);
    }
};

#endif