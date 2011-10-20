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

struct Light
{
    Vector3 position;
    Vector3 color;
};

struct Material
{
    Vector3 ambientColor;
    Vector3 diffuseColor;
    Vector3 specularColor;
    float shininess;
};

struct Transform
{
    Vector3 translation;
    Vector4 rotation;
    Vector3 scaling;
};

struct Separator
{
    std::vector<Transform> transforms;

    std::vector<Vector3> points;
    std::vector<int> indices;

    std::vector<Vector3> normals;
    std::vector<int> normalindices;

    Material material;
};

struct Scene
{
    Camera camera;
    std::vector<Separator> separators;
    std::vector<Light> lights;
};
