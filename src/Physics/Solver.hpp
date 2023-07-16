#ifndef SOLVER_H
#define SOLVER_H

#include "Entities/Circle.hpp"
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
            applyCollision();
            applyGravity();
            applyConstraints();
            applyDraw();
        }
    }

    std::vector<std::unique_ptr<VerletObject>> verletObjects;
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
        float radius = 0.02;
        for(std::unique_ptr<VerletObject>& object: verletObjects)
        {
            for(std::unique_ptr<VerletObject>& other: verletObjects)
            {
                if(object == other) continue;
                float distance = object->positionCurrent.distance(other->positionCurrent);
                if(distance < (radius)*2)
                    object->positionCurrent = object->positionCurrent - (object->positionCurrent-other->positionCurrent)*(distance-((radius)*2));
            }
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