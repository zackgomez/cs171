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
