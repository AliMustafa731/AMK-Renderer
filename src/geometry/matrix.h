#ifndef matrix_included
#define matrix_included

#include "geometry/vector.h"

struct Matrix3
{
    float m[3][3];

    // return the identity matrix
    static Matrix3 Identity();

    // fill the specified "row" of the matrix with a vector (v)
    void set_row(int _i, Vector3f v);

    // fill the specified "column" of the matrix with a vector (v)
    void set_col(int _i, Vector3f v);
    Matrix3() {}
};

struct Matrix4
{
    float m[4][4];

    // return the identity matrix
    static Matrix4 Identity();

    // fill the specified "row" of the matrix with a vector (v)
    void set_row(int _i, Vector4f v);

    // fill the specified "column" of the matrix with a vector (v)
    void set_col(int _i, Vector4f v);
    Matrix4() {}
};

// matrix multiplication
Matrix3 mul_matrix(Matrix3 m1, Matrix3 m2);
Matrix4 mul_matrix(Matrix4 m1, Matrix4 m2);

Matrix3 inverse(Matrix3 m);
Matrix3 transpose(Matrix3 m);

#endif // matrix_included