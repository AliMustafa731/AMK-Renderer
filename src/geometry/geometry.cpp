
#include "geometry/geometry.h"
#include "graphics/graphics.h"
#include <cmath>

// multiply a matrix & vector
Vector3f transform(Vector3f v, Matrix3 m)
{
    Vector3f a;

    a.x = v.x*m.m[0][0] + v.y*m.m[1][0] + v.z*m.m[2][0];
    a.y = v.x*m.m[0][1] + v.y*m.m[1][1] + v.z*m.m[2][1];
    a.z = v.x*m.m[0][2] + v.y*m.m[1][2] + v.z*m.m[2][2];

    return a;
}

// multiply a matrix & vector
Vector4f transform(Vector4f v, Matrix4 m)
{
    Vector4f a;

    a.x = v.x*m.m[0][0] + v.y*m.m[1][0] + v.z*m.m[2][0] + m.m[3][0];
    a.y = v.x*m.m[0][1] + v.y*m.m[1][1] + v.z*m.m[2][1] + m.m[3][1];
    a.z = v.x*m.m[0][2] + v.y*m.m[1][2] + v.z*m.m[2][2] + m.m[3][2];
    a.w = v.x*m.m[0][3] + v.y*m.m[1][3] + v.z*m.m[2][3] + m.m[3][3];

    return a;
}

//
// calulate the weigted coordinates of point (p) with respect to triangle (v1, v2, v3)
// for more info, see :
// https://github.com/ssloy/tinyrenderer/wiki/Lesson-2:-Triangle-rasterization-and-back-face-culling
//
Vector3f barycentric(Vector3f v1, Vector3f v2, Vector3f v3, Vector2i p)
{
    Vector3f a1(v3.x - v1.x, v2.x - v1.x, v1.x - p.x);
    Vector3f a2(v3.y - v1.y, v2.y - v1.y, v1.y - p.y);
    Vector3f r = crossProduct(a1, a2);

    r.x = r.x / r.z; // normalize
    r.y = r.y / r.z; // normalize

    return Vector3f(1.0f-r.x-r.y, r.y, r.x);
}

//
// calculate the tangent basis matrix required for "Normals mapping"
// for more info, see :
// https://github.com/ssloy/tinyrenderer/wiki/Lesson-6bis:-tangent-space-normal-mapping
//
Matrix3 TangentBasis(Face face, Vector3f n)
{
    Matrix3 A, B;

    A.set_row(0, sub(face[1].vert, face[0].vert));
    A.set_row(1, sub(face[2].vert, face[0].vert));
    A.set_row(2, n);
    A = inverse(A);

    Vector3f i = transform(Vector3f(face[1].uv.x - face[0].uv.x, face[2].uv.x - face[0].uv.x, 0), A);
    Vector3f j = transform(Vector3f(face[1].uv.y - face[0].uv.y, face[2].uv.y - face[0].uv.y, 0), A);

    B.set_col(0, normalize(i));
    B.set_col(1, normalize(j));
    B.set_col(2, n);

    return B;
}

// normalize a vector to have a magnitude of (1)
Vector3f normalize(Vector3f v)
{
    float m = sqrt((v.x*v.x) + (v.y*v.y) + (v.z*v.z));

    return Vector3f(v.x / m, v.y / m, v.z / m);
}

float dotProduct(Vector3f v1, Vector3f v2)
{
    return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

Vector3f crossProduct(Vector3f v1, Vector3f v2)
{
    return Vector3f(v2.y*v1.z - v2.z*v1.y, v2.z*v1.x - v2.x*v1.z, v2.x*v1.y - v2.y*v1.x);
}

// calculate a matrix that produces a (rotation over x-axis) transformation
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

// calculate a matrix that produces a (rotation over y-axis) transformation
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

// calculate a matrix that produces a (rotation over z-axis) transformation
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

