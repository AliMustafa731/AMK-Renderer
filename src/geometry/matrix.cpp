
#include "geometry/matrix.h"

// matrix multiplication
Matrix3 mul_matrix(Matrix3 m1, Matrix3 m2)
{
    Matrix3 _m;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            _m.m[i][j] = 0;

            for (int k = 0; k < 3; k++)
            {
                _m.m[i][j] += m1.m[i][k] * m2.m[k][j];
            }
        }
    }

    return _m;
}

// matrix multiplication
Matrix4 mul_matrix(Matrix4 m1, Matrix4 m2)
{
    Matrix4 _m;

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            _m.m[i][j] = 0;

            for (int k = 0; k < 4; k++)
            {
                _m.m[i][j] += m1.m[i][k] * m2.m[k][j];
            }
        }
    }

    return _m;
}

Matrix3 inverse(Matrix3 m)
{
    float det =
        m.m[0][0] * (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1]) -
        m.m[1][0] * (m.m[0][1] * m.m[2][2] - m.m[2][1] * m.m[0][2]) +
        m.m[2][0] * (m.m[0][1] * m.m[1][2] - m.m[1][1] * m.m[0][2]);

    Matrix3 _m; // inverse of matrix m
    _m.m[0][0] = (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1]) / det;
    _m.m[1][0] = (m.m[2][0] * m.m[1][2] - m.m[1][0] * m.m[2][2]) / det;
    _m.m[2][0] = (m.m[1][0] * m.m[2][1] - m.m[2][0] * m.m[1][1]) / det;
    _m.m[0][1] = (m.m[2][1] * m.m[0][2] - m.m[0][1] * m.m[2][2]) / det;
    _m.m[1][1] = (m.m[0][0] * m.m[2][2] - m.m[2][0] * m.m[0][2]) / det;
    _m.m[2][1] = (m.m[0][1] * m.m[2][0] - m.m[0][0] * m.m[2][1]) / det;
    _m.m[0][2] = (m.m[0][1] * m.m[1][2] - m.m[0][2] * m.m[1][1]) / det;
    _m.m[1][2] = (m.m[0][2] * m.m[1][0] - m.m[0][0] * m.m[1][2]) / det;
    _m.m[2][2] = (m.m[0][0] * m.m[1][1] - m.m[0][1] * m.m[1][0]) / det;

    return _m;
}

Matrix3 transpose(Matrix3 m)
{
    Matrix3 _m;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            _m.m[i][j] = m.m[j][i];
        }
    }

    return _m;
}

// fill the specified "column" of the matrix with a vector (v)
void Matrix3::set_col(int _i, Vector3f v)
{
    m[_i][0] = v.x;
    m[_i][1] = v.y;
    m[_i][2] = v.z;
}

// fill the specified "row" of the matrix with a vector (v)
void Matrix3::set_row(int _i, Vector3f v)
{
    m[0][_i] = v.x;
    m[1][_i] = v.y;
    m[2][_i] = v.z;
}

// fill the specified "column" of the matrix with a vector (v)
void Matrix4::set_col(int _i, Vector4f v)
{
    m[_i][0] = v.x;
    m[_i][1] = v.y;
    m[_i][2] = v.z;
    m[_i][3] = v.w;
}

// fill the specified "row" of the matrix with a vector (v)
void Matrix4::set_row(int _i, Vector4f v)
{
    m[0][_i] = v.x;
    m[1][_i] = v.y;
    m[2][_i] = v.z;
    m[3][_i] = v.w;
}

// return the identity matrix
Matrix3 Matrix3::Identity()
{
    Matrix3 m;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (i == j) { m.m[i][j] = 1.0f; }
            else { m.m[i][j] = 0; }
        }
    }

    return m;
}

// return the identity matrix
Matrix4 Matrix4::Identity()
{
    Matrix4 m;

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (i == j) { m.m[i][j] = 1.0f; }
            else { m.m[i][j] = 0; }
        }
    }

    return m;
}
