#ifndef geometry_h
#define geometry_h


template<typename T> struct Vec2
{
    T x, y;

    Vec2(T _x, T _y)
    {
        x = _x;
        y = _y;
    }

    Vec2(){}
};

template<typename T> struct Vec3
{
    T x, y, z;

    Vec3(T _x, T _y, T _z)
    {
        x = _x;
        y = _y;
        z = _z;
    }

    Vec3(){}
};

template<typename T> struct Vec4
{
    T x, y, z, w;

    Vec4(T _x, T _y, T _z, T _w)
    {
        x = _x;
        y = _y;
        z = _z;
        w = _w;
    }

    Vec4(){}
};

template<typename T> Vec2<T> add(Vec2<T> v1, Vec2<T> v2){ return Vec2<T>(v1.x+v2.x, v1.y+v2.y); }
template<typename T> Vec3<T> add(Vec3<T> v1, Vec3<T> v2){ return Vec3<T>(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z); }
template<typename T> Vec4<T> add(Vec4<T> v1, Vec4<T> v2){ return Vec4<T>(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z, v1.w+v2.w); }
template<typename T> Vec2<T> sub(Vec2<T> v1, Vec2<T> v2){ return Vec2<T>(v1.x-v2.x, v1.y-v2.y); }
template<typename T> Vec3<T> sub(Vec3<T> v1, Vec3<T> v2){ return Vec3<T>(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z); }
template<typename T> Vec4<T> sub(Vec4<T> v1, Vec4<T> v2){ return Vec4<T>(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z, v1.w-v2.w); }
template<typename T> Vec2<T> mul(Vec2<T> v1, float _x){ return Vec2<T>(v1.x*_x, v1.y*_x); }
template<typename T> Vec3<T> mul(Vec3<T> v1, float _x){ return Vec3<T>(v1.x*_x, v1.y*_x, v1.z*_x); }
template<typename T> Vec4<T> mul(Vec4<T> v1, float _x){ return Vec4<T>(v1.x*_x, v1.y*_x, v1.z*_x, v1.w*_x); }

typedef Vec2<float> Vector2;
typedef Vec3<float> Vector3;
typedef Vec4<float> Vector4;
typedef Vec2<int> Vector2i;
typedef Vec3<int> Vector3i;
typedef Vec4<int> Vector4i;

struct Matrix3
{
    float m[3][3];

    void set_row(int _i, Vector3 v);

    void set_col(int _i, Vector3 v);

    Matrix3(){}
};

struct Matrix4
{
    float m[4][4];

    void set_row(int _i, Vector4 v);

    void set_col(int _i, Vector4 v);

    Matrix4(){}
};

struct Square
{
    int x, y, w, h;

    Square(){}
    Square(int _x, int _y, int _w, int _h)
    {
        x = _x;
        y = _y;
        w = _w;
        h = _h;
    }
};

float dotProduct(Vector3 v1, Vector3 v2);
Vector3 crossProduct(Vector3 v1, Vector3 v2);
Vector3 normalize(Vector3 v);

Vector3 TransformVector(Vector3 v, Matrix3 m);
Vector4 TransformVector(Vector4 v, Matrix4 m);

Matrix3 IdentityMatrix();
Matrix3 MultiplyMatrix(Matrix3 m1, Matrix3 m2);
Matrix4 MultiplyMatrix(Matrix4 m1, Matrix4 m2);

Vector3 barycentric(Vector3* v, Vector2i p);

Matrix3 TangentBasis(Vector3 *tri, Vector2 *uv, Vector3 n);

Matrix3 inverse(Matrix3 m);

Matrix3 transpose(Matrix3 m);

Matrix3 rotation_x(float angle);
Matrix3 rotation_y(float angle);
Matrix3 rotation_z(float angle);

float length(Vector3 &v);


#endif // geometry_h
