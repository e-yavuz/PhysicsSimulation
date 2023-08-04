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

    // x(n+1) = 2*x(n)-x(n-1) + a(n)*dt^2
    //
    static void Update(vertex& positionCurrent, vertex& positionOld, vertex& acceleration, float dt)
    {
        vertex velocity = positionCurrent - positionOld;

        positionOld = positionCurrent;

        positionCurrent = positionCurrent + velocity + acceleration * (dt * dt);
    }

    static void UpdateVerticies(vertex velocity, std::pair<uint32_t, uint32_t> vertexIndicies, std::vector<float>& verticies)
    {
        for(int i = vertexIndicies.first; i < vertexIndicies.second; i+=3)
        {
            verticies[i] += velocity.x;
            verticies[i+1] += velocity.y;
            verticies[i+2] += velocity.z;
        }
    }
};

#endif