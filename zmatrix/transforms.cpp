#include "transforms.h"

// These projections are taken from:
// http://fly.srk.fer.hr/~unreal/theredbook/appendixg.html

Matrix4 make_translation(float x, float y, float z)
{
    Matrix4 res = make_identity<float, 4>();
    res(0,3) = x;
    res(1,3) = y;
    res(2,3) = z;

    return res;
}

Matrix4 make_scaling(float x, float y, float z)
{
    Matrix4 res;
    res(0,0) = x;
    res(1,1) = y;
    res(2,2) = z;
    res(3,3) = 1;

    return res;
}

Matrix4 make_rotation(float x, float y, float z, float angle)
{
    // Normalize direction vector.
    assert( !(x == 0 && y == 0 && z == 0) );
    float mag = sqrtf(x*x + y*y + z*z);
    x /= mag; y /= mag; z /= mag;

    Matrix4 res;
    res(0,0) = x*x + (1 - x*x) * cos(angle);
    res(0,1) = x*y*(1 - cos(angle)) - z*sin(angle);
    res(0,2) = x*z*(1 - cos(angle)) + y*sin(angle);

    res(1,0) = x*y*(1 - cos(angle)) + z*sin(angle);
    res(1,1) = y*y + (1 - y*y) * cos(angle);
    res(1,2) = y*z*(1 - cos(angle)) - x*sin(angle);

    res(2,0) = x*z*(1 - cos(angle)) - y*sin(angle);
    res(2,1) = y*z*(1 - cos(angle)) + x*sin(angle);
    res(2,2) = z*z + (1 - z*z) * cos(angle);

    res(3,3) = 1;

    return res;
}

Matrix4 make_perspective(float l, float r, float b, float t, float n, float f)
{
    assert(l != r && t != b && n != f);
    Matrix4 res;
    // diagonal
    res(0,0) = 2*n / (r - l);
    res(1,1) = 2*n / (t - b);
    res(2,2) = -(f + n) / (f - n);
    res(3,3) = 0.0f;

    res(0,2) = (r + l) / (r - l);
    res(1,2) = (t + b) / (t - b);
    res(3,2) = -1;

    res(2,3) = -2 * f * n / (f - n);

    return res;
}

Matrix4 make_ortho(float l, float r, float b, float t, float n, float f)
{
    assert(l != r && t != b && n != f);
    Matrix4 res;
    // diagonal
    res(0,0) = 2 / (r - l);
    res(1,1) = 2 / (t - b);
    res(2,2) = -2 / (f - n);
    res(3,3) = 1.0f;

    // right most column
    res(0,3) = -(r + l) / (r - l);
    res(1,3) = -(t + b) / (t - b);
    res(2,3) = -(f + n) / (f - n);

    return res;
}
