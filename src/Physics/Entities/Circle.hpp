#ifndef CIRCLE_H
#define CIRCLE_H

#include "VerletObject.hpp"

template<typename T>
class GraphicsContainer;

class Circle : public VerletObject
{
public:
    static void buildCircle(vertex positionCurrent, float radius, int vCount, GraphicsContainer<float>* verticies)
    {
        if(vCount < 3)
            throw std::runtime_error("Must be at least 3 vertices");

        float angle = 2 * PI / vCount;

        // int triangleCount = vCount - 2;

        vertex v0(positionCurrent.x + radius, positionCurrent.y, 0), prev(positionCurrent.x + radius * cos(angle),positionCurrent.y + radius * sin(angle),0);

        // positions
        for (int i = 2; i < vCount; i++)
        {
            verticies->push_back(v0.x);
            verticies->push_back(v0.y);
            verticies->push_back(v0.z);

            verticies->push_back(prev.x);
            verticies->push_back(prev.y);
            verticies->push_back(prev.z);

            float currentAngle = angle * i;
            float x = positionCurrent.x + radius * cos(currentAngle);
            float y = positionCurrent.y + radius * sin(currentAngle);
            float z = positionCurrent.z + 0.0f;

            verticies->push_back(x);
            verticies->push_back(y);
            verticies->push_back(z);

            prev.x = x;
            prev.y = y;
            prev.z = z;
        }
    }
};

#endif