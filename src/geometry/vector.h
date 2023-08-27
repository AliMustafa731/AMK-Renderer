#ifndef vector_included
#define vector_included


template<typename T> struct Vec2
{
    T x, y;

    Vec2(T _x, T _y)
    {
        x = _x;  y = _y;
    }
    Vec2() {}
};

template<typename T> struct Vec3
{
    T x, y, z;

    Vec3(T _x, T _y, T _z)
    {
        x = _x;  y = _y;   z = _z;
    }
    Vec3() {}
};

template<typename T> struct Vec4
{
    T x, y, z, w;

    Vec4(T _x, T _y, T _z, T _w)
    {
        x = _x;  y = _y;  z = _z;  w = _w;
    }
    Vec4() {}
};

template<typename T> Vec2<T> inline add(Vec2<T> v1, Vec2<T> v2) { return Vec2<T>(v1.x + v2.x, v1.y + v2.y); }
template<typename T> Vec3<T> inline add(Vec3<T> v1, Vec3<T> v2) { return Vec3<T>(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z); }
template<typename T> Vec4<T> inline add(Vec4<T> v1, Vec4<T> v2) { return Vec4<T>(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w); }
template<typename T> Vec2<T> inline sub(Vec2<T> v1, Vec2<T> v2) { return Vec2<T>(v1.x - v2.x, v1.y - v2.y); }
template<typename T> Vec3<T> inline sub(Vec3<T> v1, Vec3<T> v2) { return Vec3<T>(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z); }
template<typename T> Vec4<T> inline sub(Vec4<T> v1, Vec4<T> v2) { return Vec4<T>(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w); }
template<typename T> Vec2<T> inline mul(Vec2<T> v1, float _x) { return Vec2<T>(v1.x*_x, v1.y*_x); }
template<typename T> Vec3<T> inline mul(Vec3<T> v1, float _x) { return Vec3<T>(v1.x*_x, v1.y*_x, v1.z*_x); }
template<typename T> Vec4<T> inline mul(Vec4<T> v1, float _x) { return Vec4<T>(v1.x*_x, v1.y*_x, v1.z*_x, v1.w*_x); }
template<typename T> Vec2<T> inline div(Vec2<T> v1, float _x) { return Vec2<T>(v1.x / _x, v1.y / _x); }
template<typename T> Vec3<T> inline div(Vec3<T> v1, float _x) { return Vec3<T>(v1.x / _x, v1.y / _x, v1.z / _x); }
template<typename T> Vec4<T> inline div(Vec4<T> v1, float _x) { return Vec4<T>(v1.x / _x, v1.y / _x, v1.z / _x, v1.w / _x); }

typedef Vec2<float> Vector2;
typedef Vec3<float> Vector3;
typedef Vec4<float> Vector4;
typedef Vec2<int> Vector2i;
typedef Vec3<int> Vector3i;
typedef Vec4<int> Vector4i;

inline Vector4 embed_vector(Vector3 v) { return Vector4(v.x, v.y, v.z, 1.0f); }

#endif // vector_included