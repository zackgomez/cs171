/**
 * matrix.h
 *
 * author: Zack Gomez
 *
 * Simple matrix library written for Caltech's CS171.
 */
#pragma once
#include <ostream>
#include <cmath>
#include <cassert>
#include <iostream>

template<typename T, int R, int C>
class Matrix
{
public:
    // Default constructor
    Matrix()
    {
        // Zero the matrix
        for (int i = 0; i < R*C; i++)
            data_[i] = 0;
    }

    // Copy constructor
    Matrix(const Matrix &m)
    {
        if (this != &m)
            copy(m);
    }

    // Destructor
    ~Matrix() { /* empty */ }

    // Assignment operator
    const Matrix& operator=(const Matrix &rhs)
    {
        if (this != &rhs)
            copy(rhs);

        return *this;
    }

    bool operator==(const Matrix &rhs)
    {
        for (int i = 0; i < R*C; i++)
            if (data_[i] != rhs.data_[i])
                return false;
        return true;
    }

    bool operator!=(const Matrix &rhs)
    {
        return !operator==(rhs);
    }

    const T& operator()(int r, int c) const
    {
        assert(r < R && c < C && r >= 0 && c >= 0);
        return data_[r*C + c];
    }

    T& operator()(int r, int c)
    {
        assert(r < R && c < C && r >= 0 && c >= 0);
        return data_[r*C + c];
    }

    const T& operator()(int i) const
    {
        assert(i >= 0 && i < R*C);
        return data_[i];
    }

    T& operator()(int i)
    {
        assert(i >= 0 && i < R*C);
        return data_[i];
    }

    // MATRIX v SCALAR operations (+,-,*,/)
#define MAKE_MATRIX_opeq_SCALAR(func, op) \
    const Matrix& func(const T &s) \
    { \
        for (int i = 0; i < R*C; i++) \
            data_[i] op s; \
        return *this; \
    }

    MAKE_MATRIX_opeq_SCALAR(operator+=, +=)
    MAKE_MATRIX_opeq_SCALAR(operator-=, -=)
    MAKE_MATRIX_opeq_SCALAR(operator*=, *=)
    MAKE_MATRIX_opeq_SCALAR(operator/=, /=)

    // ELEMENT WISE MULTIPLICATION
    const Matrix& operator^=(const Matrix &rhs)
    {
        for (int i = 0; i < R*C; i++)
            data_[i] *= rhs.data_[i];
        return *this;
    }
    const Matrix& operator^(const Matrix &rhs) const
    {
        return Matrix(*this) ^= rhs;
    }

    /// MATRIX v MATRIX addition/subtraction
    const Matrix& operator+=(const Matrix &rhs)
    {
        for (int i = 0; i < R*C; i++)
            data_[i] += rhs.data_[i];
        return *this;
    }

    const Matrix& operator-=(const Matrix &rhs)
    {
        for (int i = 0; i < R*C; i++)
            data_[i] -= rhs.data_[i];
        return *this;
    }

    const Matrix& operator+(const Matrix &rhs) const
    {
        return Matrix(*this) += rhs;
    }

    // Returns a new matrix that is the transpose of this one
    const Matrix transpose() const
    {
        Matrix res;
        for (int r = 0; r < R; r++)
            for (int c = 0; c < C; c++)
                res.data_[r*C + c] = data_[c*R + r];

        return res;
    }

    // Only valid for n,1 dimensional matrices (column vectors)
    // Returns the magnitude squared (to avoid sqrt)
    const T magnitude2() const
    {
        assert(C == 1);
        float mag = 0;
        for (int i = 0; i < R; i++)
            mag += data_[i] * data_[i];
        return mag;
    }

    // Only valid for n,1 dimensional matrices (column vectors)
    const T magnitude() const
    {
        return sqrtf(magnitude2());
    }

    // IN PLACE normalization
    void normalize()
    {
        this->operator/=(magnitude());
    }

    // computes the dot product of two n,1 (column vector) matrices
    const T dot(const Matrix &rhs)
    {
        assert(C == 1);
        T res = 0;
        for (int i = 0; i < R; i++)
            res += data_[i] * rhs.data_[i];
        return res;
    }

    void clamp(T min, T max)
    {
        for (int i = 0; i < R*C; i++)
        {
            data_[i] = data_[i] > max ? max : data_[i];
            data_[i] = data_[i] < min ? min : data_[i];
        }
    }

    // Returns the inverse of this matrix
    const Matrix inverse() const;

private:
    // The data stored in a one dimensional array
    T data_[R * C];

    // Copies the data from the matrix into this one
    void copy(const Matrix &m)
    {
        for (int i = 0; i < R*C; i++)
            data_[i] = m.data_[i];
    }
};

template<typename T, int R, int C>
const Matrix<T,R,C> Matrix<T,R,C>::inverse() const
{
    // Sanity check- only square matrices
    assert(R == C);
    // Create temporary identity augmented matrix
    Matrix<T,R,2*C> temp;
    for (int r = 0; r < R; r++)
    {
        for (int c = 0; c < C; c++)
            temp(r, c) = data_[r*C + c];
        for (int c = C; c < 2*C; c++)
            temp(r, c) = (c - C == r) ? 1 : 0;
    }

    // Algorithm taken from wikipedia page on gaussian elimination
    // Uses partial pivoting to improve numerical stability
    int i = 0, j = 0;
    while (i < R && j < 2*C)
    {
        // Find the pivot in column j, starting with element in row i
        int maxi = i;
        for (int k = i+1; k < R; k++)
            if (fabs(temp(k, j)) > fabs(temp(maxi, j)))
                maxi = k;

        if (temp(maxi, j) != 0)
        {
            // Swap rows i and maxi
            for (int k = 0; k < 2*C; k++)
            {
                T tval = temp(i, k);
                temp(i, k) = temp(maxi, k);
                temp(maxi, k) = tval;
            }
            // Divide row i by temp(i, j)
            T val = temp(i, j);
            for (int k = 0; k < 2*C; k++)
                temp(i, k) /= val;

            // Subtract from other rows
            for (int u = 0; u < R; u++)
            {
                val = temp(u, j);
                if (u == i) continue;
                for (int k = 0; k < 2*C; k++)
                    temp(u, k) -= val * temp(i, k);
            }
            ++i;
        }
        ++j;
    }


    // Finally construct the inverse matrix from the right hand side of the 
    // augmented matrix
    Matrix<T,R,C> res;
    for (int r = 0; r < R; r++)
        for (int c = 0; c < C; c++)
            res(r, c) = temp(r, c+C);

    return res;
}

// FREE FUNCTIONS FOLLOW

#define MAKE_MATRIX_op_SCALAR(func, opeq) \
template<typename T, int R, int C> \
const Matrix<T,R,C> func(const Matrix<T,R,C>& mat, const T& s) \
{\
    Matrix<T,R,C> res(mat); \
    res opeq s; \
    return res; \
}

MAKE_MATRIX_op_SCALAR(operator+, +=)
MAKE_MATRIX_op_SCALAR(operator-, -=)
MAKE_MATRIX_op_SCALAR(operator*, *=)
MAKE_MATRIX_op_SCALAR(operator/, /=)

template<typename T, int CR, int ANY1, int ANY2>
const T rdotc(const Matrix<T,ANY1,CR> &m1, int row, const Matrix<T,CR,ANY2> &m2, int col)
{
    T res = 0;
    for (int i = 0; i < CR; i++)
        res += m1(row, i) * m2(i, col);

    return res;
}

template<typename T, int R, int CR, int C>
const Matrix<T,R,C> operator*(const Matrix<T,R,CR> &m1, const Matrix<T,CR,C> &m2)
{
    Matrix<T,R,C> res;
    for (int r = 0; r < R; r++)
        for (int c = 0; c < C; c++)
            res(r, c) = rdotc(m1, r, m2, c);
    return res;
}

template<typename T, int R, int C>
std::ostream& operator<<(std::ostream& os, const Matrix<T, R, C> &m)
{
    os << '[';
    for (int r = 0; r < R; r++)
    {
        if (r)
            os << "\n ";
        os << '[';
        for (int c = 0; c < C; c++)
        {
            if (c)
                os << ' ';
            os << m(r, c);
        }
        os << "]";
    }
    os << "]\n";

    return os;
}

template<typename T, int N>
const Matrix<T,N,N> make_identity()
{
    Matrix<T,N,N> res;
    for (int i = 0; i < N; i++)
        res(i,i) = 1;

    return res;
}

typedef float data_t;
typedef Matrix<data_t, 3, 1> Vector3;
typedef Matrix<data_t, 4, 1> Vector4;
typedef Matrix<data_t, 3, 3> Matrix3;
typedef Matrix<data_t, 4, 4> Matrix4;

// Vector functions
Vector3 makeVector3(data_t x, data_t y, data_t z);
Vector4 makeVector4(data_t x, data_t y, data_t z, data_t w);
Vector4 homogenize(const Vector3&);

