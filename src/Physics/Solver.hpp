#ifndef SOLVER_H
#define SOLVER_H

#include "Entities/Circle.hpp"
#include "Entities/VerletObject.hpp"
#include <memory>
#include <unordered_map>
#include <utility>

template<typename T>
class VertexContainer;

#define gravity vertex(0, -0.001f, 0)

class Solver{
public:
    void Update(int subSteps)
    {
        for(int i = 0; i < subSteps; i++)
        {
            applyUpdate(1.0f/subSteps);
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
    std::vector<std::pair<uint32_t, uint32_t>> objectsVertexIndicies;
    VertexContainer<float>* vertices;

    constexpr const static float spatialHashGranularity = 0.025;
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
        // applySpatialHash();
        // for(auto& entry: spatialHash)
        // {
        //     int x = entry.first.first;
        //     int y = entry.first.second;
        //     for(size_t objectIndex: entry.second)
        //     {
        //         std::unique_ptr<VerletObject>& object = verletObjects[objectIndex];
        //         checkSpatialHash(x, y, object);

        //         checkSpatialHash(x, y+1, object);
        //         checkSpatialHash(x, y-1, object);

        //         checkSpatialHash(x+1, y, object);
        //         checkSpatialHash(x-1, y, object);

        //         checkSpatialHash(x+1, y+1, object);
        //         checkSpatialHash(x-1, y+1, object);

        //         checkSpatialHash(x+1, y-1, object);
        //         checkSpatialHash(x-1, y-1, object);
        //     }
        // }

        // O(n^2) solution
        for(uint32_t i = 0; i < objectsCurrentPosition.size(); i++)
            for(uint32_t j = 0; j < objectsCurrentPosition.size(); j++)
                checkCollision(i, j);
    }

    // void checkSpatialHash(int x, int y, std::unique_ptr<VerletObject>& object)
    // {
    //     if(spatialHash.find({x,y}) == spatialHash.end()) return;

    //     std::vector<size_t>& otherVector = spatialHash[{x,y}];

    //     for(int i = 0; i < spatialHash[{x,y}].size(); i++)
    //         checkCollision(object, verletObjects[spatialHash[{x,y}][i]]);
    // }

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

    // void applySpatialHash()
    // {
    //     spatialHash.clear();

    //     //Parse through verletObjects, adding (usually re-adding) the objects into the hashmap
    //     for(size_t i = 0; i < verletObjects.size(); i++)
    //     {
    //         int x = int(verletObjects[i]->positionCurrent.x/spatialHashGranularity);
    //         int y = int(verletObjects[i]->positionCurrent.y/spatialHashGranularity);

    //         if(spatialHash.find({x, y}) != spatialHash.end())
    //             spatialHash[{x,y}].push_back(i);
    //         else
    //             spatialHash[{x,y}] = std::vector<size_t>{i};
    //     }
    // }

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
            objectsAcceleration[i] = gravity;
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