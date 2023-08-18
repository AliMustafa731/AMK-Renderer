#include "geometry.h"
#include <cmath>


void Matrix3::set_col(int _i, Vector3 v)
{
    m[_i][0] = v.x;
    m[_i][1] = v.y;
    m[_i][2] = v.z;
}

void Matrix3::set_row(int _i, Vector3 v)
{
    m[0][_i] = v.x;
    m[1][_i] = v.y;
    m[2][_i] = v.z;
}

void Matrix4::set_col(int _i, Vector4 v)
{
    m[_i][0] = v.x;
    m[_i][1] = v.y;
    m[_i][2] = v.z;
    m[_i][3] = v.w;
}

void Matrix4::set_row(int _i, Vector4 v)
{
    m[0][_i] = v.x;
    m[1][_i] = v.y;
    m[2][_i] = v.z;
    m[3][_i] = v.w;
}

Vector3 TransformVector(Vector3 v, Matrix3 m)
{
    Vector3 a;

    a.x = v.x*m.m[0][0] + v.y*m.m[1][0] + v.z*m.m[2][0];
    a.y = v.x*m.m[0][1] + v.y*m.m[1][1] + v.z*m.m[2][1];
    a.z = v.x*m.m[0][2] + v.y*m.m[1][2] + v.z*m.m[2][2];

    return a;
}
Vector4 TransformVector(Vector4 v, Matrix4 m)
{
    Vector4 a;

    a.x = v.x*m.m[0][0] + v.y*m.m[1][0] + v.z*m.m[2][0] + m.m[3][0];
    a.y = v.x*m.m[0][1] + v.y*m.m[1][1] + v.z*m.m[2][1] + m.m[3][1];
    a.z = v.x*m.m[0][2] + v.y*m.m[1][2] + v.z*m.m[2][2] + m.m[3][2];
    a.w = v.x*m.m[0][3] + v.y*m.m[1][3] + v.z*m.m[2][3] + m.m[3][3];

    return a;
}

Matrix3 IdentityMatrix()
{
    Matrix3 _m;

    for(int i = 0 ; i < 3 ; i++)
    {
        for(int j = 0 ; j < 3 ; j++)
        {
            if(i == j)
            {
                _m.m[i][j] = 1;
            }
            else
            {
                _m.m[i][j] = 0;
            }
        }
    }

    return _m;
}

Matrix3 MultiplyMatrix(Matrix3 m1, Matrix3 m2)
{
    Matrix3 _m;

    for(int i = 0 ; i < 3 ; i++)
    {
        for(int j = 0 ; j < 3 ; j++)
        {
            _m.m[i][j] = 0;

            for(int k = 0 ; k < 3 ; k++)
            {
                _m.m[i][j] += m1.m[i][k]*m2.m[k][j];
            }
        }
    }

    return _m;
}
Matrix4 MultiplyMatrix(Matrix4 m1, Matrix4 m2)
{
    Matrix4 _m;

    for(int i = 0 ; i < 4 ; i++)
    {
        for(int j = 0 ; j < 4 ; j++)
        {
            _m.m[i][j] = 0;

            for(int k = 0 ; k < 4 ; k++)
            {
                _m.m[i][j] += m1.m[i][k]*m2.m[k][j];
            }
        }
    }

    return _m;
}

Vector3 barycentric(Vector3* v, Vector2i p)
{
    Vector3 v1(v[2].x - v[0].x, v[1].x - v[0].x, v[0].x - p.x);
    Vector3 v2(v[2].y - v[0].y, v[1].y - v[0].y, v[0].y - p.y);
    Vector3 r = crossProduct(v1, v2);

    r.x = r.x / r.z; // normalize
    r.y = r.y / r.z; // normalize

    return Vector3(1.0-r.x-r.y, r.y, r.x);
}

Matrix3 TangentBasis(Vector3 *tri, Vector2 *uv, Vector3 n)
{
    Matrix3 A, B;

    A.set_row(0, sub(tri[1], tri[0]));
    A.set_row(1, sub(tri[2], tri[0]));
    A.set_row(2, n);
    A = inverse(A);

    Vector3 i = TransformVector(Vector3(uv[1].x - uv[0].x, uv[2].x - uv[0].x, 0), A);
    Vector3 j = TransformVector(Vector3(uv[1].y - uv[0].y, uv[2].y - uv[0].y, 0), A);

    B.set_col(0, normalize(i));
    B.set_col(1, normalize(j));
    B.set_col(2, n);

    return B;
}

Matrix3 inverse(Matrix3 m)
{
    float det = m.m[0][0] * (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1]) -
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

    for(int i = 0 ; i < 3 ; i++)
    {
        for(int j = 0 ; j < 3 ; j++)
        {
            _m.m[i][j] = m.m[j][i];
        }
    }

    return _m;
}

Vector3 normalize(Vector3 v)
{
    float m = sqrt((v.x*v.x) + (v.y*v.y) + (v.z*v.z));

    return Vector3(v.x / m, v.y / m, v.z / m);
}

float dotProduct(Vector3 v1, Vector3 v2)
{
    return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

Vector3 crossProduct(Vector3 v1, Vector3 v2)
{
    return Vector3(v2.y*v1.z - v2.z*v1.y, v2.z*v1.x - v2.x*v1.z, v2.x*v1.y - v2.y*v1.x);
}

float length(Vector3 &v)
{
    return sqrt((v.x*v.x) + (v.y*v.y) + (v.z*v.z));
}


Matrix3 rotation_x(float angle)
{
    Matrix3 _m = IdentityMatrix();
    float sin = std::sin(angle);
    float cos = std::cos(angle);

    _m.m[1][1] = cos;
    _m.m[1][2] = sin;
    _m.m[2][1] = -sin;
    _m.m[2][2] = cos;

    return _m;
}

Matrix3 rotation_y(float angle)
{
    Matrix3 _m = IdentityMatrix();
    float sin = std::sin(angle);
    float cos = std::cos(angle);

    _m.m[0][0] = cos;
    _m.m[0][2] = -sin;
    _m.m[2][0] = sin;
    _m.m[2][2] = cos;

    return _m;
}

Matrix3 rotation_z(float angle)
{
    Matrix3 _m = IdentityMatrix();
    float sin = std::sin(angle);
    float cos = std::cos(angle);

    _m.m[0][0] = cos;
    _m.m[0][1] = sin;
    _m.m[1][0] = -sin;
    _m.m[1][1] = cos;

    return _m;
}


