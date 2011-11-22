#pragma once
#include <vector>
#include <glm/glm.hpp>

struct keyframe
{
    int framenum;
    glm::vec3 translation;
    glm::vec3 scale;
    glm::vec4 rotation; // vec3 vec, float angle
};

struct animation
{
    int nframes;
    std::vector<keyframe> keyframes;
};
