#pragma once
#include "matrix.h"

Matrix4 make_translation(float x, float y, float z);

Matrix4 make_scaling(float x, float y, float z);

Matrix4 make_rotation(float x, float y, float z, float angle);

Matrix4 make_ortho(float l, float r, float b, float t, float n, float f);
