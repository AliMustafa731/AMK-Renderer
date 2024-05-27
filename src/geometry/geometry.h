#ifndef geometry_h
#define geometry_h

#include "geometry/matrix.h"
#include "geometry/vector.h"

// multiply a matrix & vector
Vector3f transform(Vector3f v, Matrix3 m);
Vector4f transform(Vector4f v, Matrix4 m);

float dotProduct(Vector3f v1, Vector3f v2);
Vector3f crossProduct(Vector3f v1, Vector3f v2);

// normalize a vector to have a magnitude of (1)
Vector3f normalize(Vector3f v);

//
// calulate the weigted coordinates of point (p) with respect to triangle (v1, v2, v3)
// for more info, see :
// https://github.com/ssloy/tinyrenderer/wiki/Lesson-2:-Triangle-rasterization-and-back-face-culling
//
Vector3f barycentric(Vector3f v1, Vector3f v2, Vector3f v3, Vector2i p);

struct Face;

//
// calculate the tangent basis matrix required for "Normals mapping"
// for more info, see :
// https://github.com/ssloy/tinyrenderer/wiki/Lesson-6bis:-tangent-space-normal-mapping
//
Matrix3 TangentBasis(Face face, Vector3f n);

// calculate a matrix that produces a (rotation over an axis) transformation
Matrix3 rotation_x(float angle);
Matrix3 rotation_y(float angle);
Matrix3 rotation_z(float angle);

#endif // geometry_h
