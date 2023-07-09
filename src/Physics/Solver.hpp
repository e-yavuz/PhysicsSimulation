#ifndef SOLVER_H
#define SOLVER_H

#include "Entities/VerletObject.hpp"
#include <memory>

#define gravity vertex(0, -0.001f, 0)

class Solver{
public:
    void Update(int subSteps)
    {
        for(int i = 0; i < subSteps; i++)
        {
            applyUpdate(1.0f/subSteps);
            applyGravity();
            applyConstraints();
            applyDraw();
        }
    }

    void applyConstraints()
    {
        vertex center(0,0,0);
        float radius = 0.5f;
        for(std::unique_ptr<VerletObject>& object: verletObjects)
        {
            vertex diff = object->positionCurrent-center;
            float distance = center.distance(object->positionCurrent);
            if(distance > radius)
                object->positionCurrent = object->positionCurrent - diff*(distance-radius);
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
    std::vector<std::unique_ptr<VerletObject>> verletObjects;
};

#endif