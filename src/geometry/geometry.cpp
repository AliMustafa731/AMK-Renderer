
#include "geometry/geometry.h"
#include "graphics/graphics.h"
#include <cmath>


Vector3 transform(Vector3 v, Matrix3 m)
{
    Vector3 a;

    a.x = v.x*m.m[0][0] + v.y*m.m[1][0] + v.z*m.m[2][0];
    a.y = v.x*m.m[0][1] + v.y*m.m[1][1] + v.z*m.m[2][1];
    a.z = v.x*m.m[0][2] + v.y*m.m[1][2] + v.z*m.m[2][2];

    return a;
}

Vector4 transform(Vector4 v, Matrix4 m)
{
    Vector4 a;

    a.x = v.x*m.m[0][0] + v.y*m.m[1][0] + v.z*m.m[2][0] + m.m[3][0];
    a.y = v.x*m.m[0][1] + v.y*m.m[1][1] + v.z*m.m[2][1] + m.m[3][1];
    a.z = v.x*m.m[0][2] + v.y*m.m[1][2] + v.z*m.m[2][2] + m.m[3][2];
    a.w = v.x*m.m[0][3] + v.y*m.m[1][3] + v.z*m.m[2][3] + m.m[3][3];

    return a;
}

Vector3 barycentric(Vector3* v, Vector2i p)
{
    Vector3 v1(v[2].x - v[0].x, v[1].x - v[0].x, v[0].x - p.x);
    Vector3 v2(v[2].y - v[0].y, v[1].y - v[0].y, v[0].y - p.y);
    Vector3 r = crossProduct(v1, v2);

    r.x = r.x / r.z; // normalize
    r.y = r.y / r.z; // normalize

    return Vector3(1.0f-r.x-r.y, r.y, r.x);
}

Matrix3 TangentBasis(Face face, Vector3 n)
{
    Matrix3 A, B;

    A.set_row(0, sub(face[1].vert, face[0].vert));
    A.set_row(1, sub(face[2].vert, face[0].vert));
    A.set_row(2, n);
    A = inverse(A);

    Vector3 i = transform(Vector3(face[1].uv.x - face[0].uv.x, face[2].uv.x - face[0].uv.x, 0), A);
    Vector3 j = transform(Vector3(face[1].uv.y - face[0].uv.y, face[2].uv.y - face[0].uv.y, 0), A);

    B.set_col(0, normalize(i));
    B.set_col(1, normalize(j));
    B.set_col(2, n);

    return B;
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
    Matrix3 _m = Matrix3::Identity();
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
    Matrix3 _m = Matrix3::Identity();
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
    Matrix3 _m = Matrix3::Identity();
    float sin = std::sin(angle);
    float cos = std::cos(angle);

    _m.m[0][0] = cos;
    _m.m[0][1] = sin;
    _m.m[1][0] = -sin;
    _m.m[1][1] = cos;

    return _m;
}

