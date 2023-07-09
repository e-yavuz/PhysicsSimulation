#ifndef VERLETOBJECT_H
#define VERLETOBJECT_H

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

    float distance(vertex other)
    {
        float diff_x = (x-other.x)*(x-other.x);
        float diff_y = (y-other.y)*(y-other.y);
        float diff_z = (z-other.z)*(z-other.z);

        return sqrt(diff_x+diff_y+diff_z);
    }

    float x;
    float y;
    float z;
};

class VerletObject
{
public:
    VerletObject(float x, float y, float z) : positionCurrent(x, y, z), positionOld(x, y, z), acceleration(0,0,0) {}
    virtual ~VerletObject() {}

    // x(n+1) = 2*x(n)-x(n-1) + a(n)*dt^2
    //
    void Update(float dt)
    {
        vertex velocity = positionCurrent - positionOld;

        positionOld = positionCurrent;

        positionCurrent = positionCurrent + velocity + acceleration * (dt * dt);
    }

    virtual void UpdateVerticies() = 0;

    vertex positionCurrent;
    vertex positionOld;
    vertex acceleration;
};

#endif