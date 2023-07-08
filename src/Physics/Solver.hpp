#ifndef SOLVER_H
#define SOLVER_H

#include "Entities/Circle.hpp"

#define gravity vertex(0, -0.000002f, 0)

class Solver{
public:
    void Update()
    {
        for(VerletObject* object: verletObjects)
        {
            object->Update();
            object->acceleration = gravity;
        }
    }
    std::vector<VerletObject*> verletObjects;
};

#endif