#ifndef CIRCLE_H
#define CIRCLE_H

#include "VerletObject.hpp"

class Circle : public VerletObject
{
public:
    Circle(float x, float y, float z, float radius, int vCount, std::vector<float>& vertices) : VerletObject(x, y, z), p_verticies(&vertices)
    {
        this->radius = radius;
        start_ind = p_verticies->size();
        buildCircle(radius, vCount);
        end_ind = p_verticies->size();
    }

    ~Circle() {}

    // Circle() : VerletObject(0,0,0), p_verticies(nullptr) {}

    void buildCircle(float radius, int vCount)
    {
        if(vCount < 3)
            throw std::runtime_error("Must be at least 3 vertices");

        float angle = 2 * PI / vCount;

        // int triangleCount = vCount - 2;

        vertex v0(positionCurrent.x + radius, positionCurrent.y, 0), prev(positionCurrent.x + radius * cos(angle),positionCurrent.y + radius * sin(angle),0);

        // positions
        for (int i = 2; i < vCount; i++)
        {
            p_verticies->push_back(v0.x);
            p_verticies->push_back(v0.y);
            p_verticies->push_back(v0.z);

            p_verticies->push_back(prev.x);
            p_verticies->push_back(prev.y);
            p_verticies->push_back(prev.z);

            float currentAngle = angle * i;
            float x = positionCurrent.x + radius * cos(currentAngle);
            float y = positionCurrent.y + radius * sin(currentAngle);
            float z = positionCurrent.z + 0.0f;

            p_verticies->push_back(x);
            p_verticies->push_back(y);
            p_verticies->push_back(z);

            prev.x = x;
            prev.y = y;
            prev.z = z;
        }
    }

    void UpdateVerticies()
    {
        vertex velocity = positionCurrent-positionOld;
        for(int i = start_ind; i < end_ind; i+=3)
        {
            (*p_verticies)[i] += velocity.x;
            (*p_verticies)[i+1] += velocity.y;
            (*p_verticies)[i+2] += velocity.z;
        }
    }

    float radius;
    size_t start_ind;
    size_t end_ind;
    std::vector<float>* p_verticies;
};

#endif