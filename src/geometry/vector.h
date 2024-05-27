#ifndef vector_included
#define vector_included

#include <cmath>  // for std::sqrt()

// forward declarations
template<typename T> struct Vector2;
template<typename T> struct Vector3;
template<typename T> struct Vector4;

template<typename T> struct Vector2
{
    T x, y;

    Vector2(T _x, T _y) : x(_x), y(_y) {}
    Vector2(Vector3<T> v);
    Vector2(Vector4<T> v);
    Vector2() {}

    float length()
    {
        return std::sqrt(x * x + y * y);
    }
};

template<typename T> struct Vector3
{
    T x, y, z;

    Vector3(T _x, T _y, T _z) : x(_x), y(_y), z(_z) {}
    Vector3(Vector2<T> v);
    Vector3(Vector4<T> v);
    Vector3() {}

    float length()
    {
        return std::sqrt(x * x + y * y + z * z);
    }
};

template<typename T> struct Vector4
{
    T x, y, z, w;

    Vector4(T _x, T _y, T _z, T _w) : x(_x), y(_y), z(_z), w(_w) {}
    Vector4(Vector2<T> v);
    Vector4(Vector3<T> v);
    Vector4() {}

    float length()
    {
        return std::sqrt(x * x + y * y + z * z + w * w);
    }
};

// embedding vectors of different sizes
template<typename T> Vector2<T>::Vector2(Vector3<T> v) : x(v.x), y(v.y) {}
template<typename T> Vector2<T>::Vector2(Vector4<T> v) : x(v.x), y(v.y) {}

template<typename T> Vector3<T>::Vector3(Vector2<T> v) : x(v.x), y(v.y), z(0) {}
template<typename T> Vector3<T>::Vector3(Vector4<T> v) : x(v.x), y(v.y), z(v.z) {}

template<typename T> Vector4<T>::Vector4(Vector2<T> v) : x(v.x), y(v.y), z(0), w(0) {}
template<typename T> Vector4<T>::Vector4(Vector3<T> v) : x(v.x), y(v.y), z(v.z), w(1.0f) {}

// arithmetic operations
template<typename T> Vector2<T> inline add(Vector2<T> v1, Vector2<T> v2) { return Vector2<T>(v1.x + v2.x, v1.y + v2.y); }
template<typename T> Vector3<T> inline add(Vector3<T> v1, Vector3<T> v2) { return Vector3<T>(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z); }
template<typename T> Vector4<T> inline add(Vector4<T> v1, Vector4<T> v2) { return Vector4<T>(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w); }

template<typename T> Vector2<T> inline sub(Vector2<T> v1, Vector2<T> v2) { return Vector2<T>(v1.x - v2.x, v1.y - v2.y); }
template<typename T> Vector3<T> inline sub(Vector3<T> v1, Vector3<T> v2) { return Vector3<T>(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z); }
template<typename T> Vector4<T> inline sub(Vector4<T> v1, Vector4<T> v2) { return Vector4<T>(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w); }

template<typename T> Vector2<T> inline mul(Vector2<T> v1, float _x) { return Vector2<T>(v1.x*_x, v1.y*_x); }
template<typename T> Vector3<T> inline mul(Vector3<T> v1, float _x) { return Vector3<T>(v1.x*_x, v1.y*_x, v1.z*_x); }
template<typename T> Vector4<T> inline mul(Vector4<T> v1, float _x) { return Vector4<T>(v1.x*_x, v1.y*_x, v1.z*_x, v1.w*_x); }

template<typename T> Vector2<T> inline div(Vector2<T> v1, float _x) { return Vector2<T>(v1.x / _x, v1.y / _x); }
template<typename T> Vector3<T> inline div(Vector3<T> v1, float _x) { return Vector3<T>(v1.x / _x, v1.y / _x, v1.z / _x); }
template<typename T> Vector4<T> inline div(Vector4<T> v1, float _x) { return Vector4<T>(v1.x / _x, v1.y / _x, v1.z / _x, v1.w / _x); }

// type definitions
typedef Vector2<float> Vector2f;
typedef Vector3<float> Vector3f;
typedef Vector4<float> Vector4f;
typedef Vector2<int> Vector2i;
typedef Vector3<int> Vector3i;
typedef Vector4<int> Vector4i;

#endif // vector_included