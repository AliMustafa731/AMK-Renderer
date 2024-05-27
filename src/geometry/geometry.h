#ifndef geometry_h
#define geometry_h

#include "geometry/matrix.h"
#include "geometry/vector.h"

Vector3 transform(Vector3 v, Matrix3 m);
Vector4 transform(Vector4 v, Matrix4 m);

float dotProduct(Vector3 v1, Vector3 v2);
Vector3 crossProduct(Vector3 v1, Vector3 v2);
Vector3 normalize(Vector3 v);

Vector3 barycentric(Vector3 v1, Vector3 v2, Vector3 v3, Vector2i p);

struct Face;

Matrix3 TangentBasis(Face face, Vector3 n);

Matrix3 rotation_x(float angle);
Matrix3 rotation_y(float angle);
Matrix3 rotation_z(float angle);

float length(Vector3 &v);


#endif // geometry_h
