#ifndef matrix_included
#define matrix_included

#include "geometry/vector.h"

struct Matrix3
{
    float m[3][3];

    static Matrix3 Identity();
    void set_row(int _i, Vector3 v);
    void set_col(int _i, Vector3 v);
    Matrix3() {}
};

struct Matrix4
{
    float m[4][4];

    static Matrix4 Identity();
    void set_row(int _i, Vector4 v);
    void set_col(int _i, Vector4 v);
    Matrix4() {}
};

Matrix3 mul_matrix(Matrix3 m1, Matrix3 m2);
Matrix4 mul_matrix(Matrix4 m1, Matrix4 m2);

Matrix3 inverse(Matrix3 m);
Matrix3 transpose(Matrix3 m);

#endif // matrix_included