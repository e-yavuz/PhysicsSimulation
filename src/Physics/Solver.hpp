#ifndef SOLVER_H
#define SOLVER_H

#include "Entities/Circle.hpp"
#include "Entities/VerletObject.hpp"
#include <memory>
#include <unordered_map>
#include <utility>

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

    template<typename T1>
    void addObject(T1& object)
    {
        verletObjects.emplace_back(std::make_unique<T1>(object));
    }

    struct CustomHash {
        template <typename T1, typename T2>
        inline auto operator()(const std::pair<T1, T2> &p) const -> size_t 
        {
            return p.first*6557 + p.second;
        }
    };

    std::vector<std::unique_ptr<VerletObject>> verletObjects;
    std::unordered_map<std::pair<int, int>, std::vector<size_t>, CustomHash> spatialHash;
    constexpr const static float spatialHashGranularity = 0.025;
private:

    void applyConstraints()
    {
        vertex center(0,0,0);
        for(std::unique_ptr<VerletObject>& object: verletObjects)
        {
            Circle* c = dynamic_cast<Circle*>(object.get());
            vertex& current = object->positionCurrent;
            float boundary = 1.0f-c->radius;

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
        applySpatialHash();
        for(auto& entry: spatialHash)
        {
            int x = entry.first.first;
            int y = entry.first.second;
            for(size_t objectIndex: entry.second)
            {
                std::unique_ptr<VerletObject>& object = verletObjects[objectIndex];
                checkSpatialHash(x, y, object);

                checkSpatialHash(x, y+1, object);
                checkSpatialHash(x, y-1, object);

                checkSpatialHash(x+1, y, object);
                checkSpatialHash(x-1, y, object);

                checkSpatialHash(x+1, y+1, object);
                checkSpatialHash(x-1, y+1, object);

                checkSpatialHash(x+1, y-1, object);
                checkSpatialHash(x-1, y-1, object);
            }
        }

        // O(n^2) solution
        // for(std::unique_ptr<VerletObject>& object: verletObjects)
        //     for(std::unique_ptr<VerletObject>& other: verletObjects)
        //         checkCollision(object, other);
    }

    void checkSpatialHash(int x, int y, std::unique_ptr<VerletObject>& object)
    {
        if(spatialHash.find({x,y}) == spatialHash.end()) return;

        std::vector<size_t>& otherVector = spatialHash[{x,y}];

        for(int i = 0; i < spatialHash[{x,y}].size(); i++)
            checkCollision(object, verletObjects[spatialHash[{x,y}][i]]);
    }

    inline void checkCollision(std::unique_ptr<VerletObject>& object, std::unique_ptr<VerletObject>& other)
    {
        if(object == other) return;
        float radius = dynamic_cast<Circle*>(object.get())->radius;

        float distance = object->positionCurrent.distance(other->positionCurrent);

        if(distance < (radius)*2)
            object->positionCurrent = object->positionCurrent - (object->positionCurrent-other->positionCurrent)*(distance-((radius)*2));
    }

    void applySpatialHash()
    {
        spatialHash.clear();

        //Parse through verletObjects, adding (usually re-adding) the objects into the hashmap
        for(size_t i = 0; i < verletObjects.size(); i++)
        {
            int x = int(verletObjects[i]->positionCurrent.x/spatialHashGranularity);
            int y = int(verletObjects[i]->positionCurrent.y/spatialHashGranularity);

            if(spatialHash.find({x, y}) != spatialHash.end())
                spatialHash[{x,y}].push_back(i);
            else
                spatialHash[{x,y}] = std::vector<size_t>{i};
        }
    }

    void applyDraw()
    {
        for(std::unique_ptr<VerletObject>& object: verletObjects)
                object->UpdateVerticies();
    }

    void applyGravity()
    {
        for(std::unique_ptr<VerletObject>& object: verletObjects)
            object->acceleration = gravity;
    }
    void applyUpdate(float dt)
    {
        for(std::unique_ptr<VerletObject>& object: verletObjects)
            object->Update(dt);
    }
};

#endif