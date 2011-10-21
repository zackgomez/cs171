#include "matrix.h"

Vector3 makeVector3(data_t x, data_t y, data_t z)
{
    Vector3 ret;
    ret(0) = x;
    ret(1) = y;
    ret(2) = z;

    return ret;
}

Vector4 makeVector4(data_t x, data_t y, data_t z, data_t w)
{
    Vector4 ret;
    ret(0) = x;
    ret(1) = y;
    ret(2) = z;
    ret(3) = w;

    return ret;
}

Vector4 homogenize(const Vector3 &v)
{
    Vector4 ret;
    ret(0) = v(0);
    ret(1) = v(1);
    ret(2) = v(2);
    ret(3) = 1;

    return ret;
}
