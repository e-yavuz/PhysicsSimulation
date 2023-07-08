#ifndef CIRCLE_H
#define CIRCLE_H

#include <vector>
#include <math.h>

#define PI 3.1415f

struct vertex
{
    vertex(float x, float y, float z) : x(x), y(y), z(z) {}
    vertex() : x(0), y(0), z(0) {}

    vertex operator+(vertex const& rhs)
    {
        return vertex(x + rhs.x, y + rhs.y, z + rhs.z);
    }
    vertex operator-(vertex const& rhs)
    {
        return vertex(x - rhs.x, y - rhs.y, z - rhs.z);
    }
    vertex operator-()
    {
        return vertex(-x, -y, -z);
    }
    vertex operator*(float const& rhs)
    {
        return vertex(x*rhs, y*rhs, z*rhs);
    }

    float x;
    float y;
    float z;
};

class VerletObject
{
public:
    VerletObject(float x, float y, float z) : positionCurrent(x, y, z), positionOld(x, y, z), acceleration(0,0,0) {}

    // x(n+1) = 2*x(n)-x(n-1) + a(n)*dt^2
    //
    void Update()
    {
        vertex velocity = positionCurrent - positionOld;

        positionOld = positionCurrent;

        positionCurrent = positionCurrent + velocity + acceleration * (dt * dt);

        UpdateVerticies();
    }

    virtual void UpdateVerticies() = 0;

    vertex positionCurrent;
    vertex positionOld;
    vertex acceleration;
    float dt = 1;
};

class Circle : public VerletObject
{
public:
    Circle(float x, float y, float z, float radius, int vCount, std::vector<float>& vertices) : VerletObject(x, y, z), p_verticies(&vertices)
    {
        start_ind = p_verticies->size();
        buildCircle(radius, vCount);
        end_ind = p_verticies->size();
    }

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

    size_t start_ind;
    size_t end_ind;
    std::vector<float>* p_verticies;
};

#endif