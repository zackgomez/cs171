#pragma once
#include "matrix.h"
#include <vector>

struct Camera
{
    Vector3 position;
    Vector4 orientation;
    float nearDistance;
    float farDistance;
    float left;
    float right;
    float top;
    float bottom;
};

struct Separator
{
    Matrix4 transform;
    std::vector<Vector3> points;
    std::vector<int> indices;
};

struct Scene
{
    Camera camera;
    std::vector<Separator> separators;
};
