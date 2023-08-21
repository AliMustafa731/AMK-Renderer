#ifndef graphics_h
#define graphics_h

#include "geometry.h"
#include "array.h"


struct Color
{
    unsigned char b, g, r, a;

    Color(){}
    Color(unsigned char _r, unsigned char _g, unsigned char _b)
    {
        r = _r;  g = _g;  b = _b;
    }
};

typedef Buffer<Color> FrameBuffer;
typedef Buffer<float> ZBuffer;

struct Triangle
{
    Vector3 vert[3];
    Vector2 uv[3];
    Vector3 norm[3];

    Triangle(){}
};

struct VertexInfo
{
    int vert, uv, norm; // index

    VertexInfo(){}
};

struct Face
{
    VertexInfo v[3];

    Face(){}

    inline VertexInfo operator[](int i) const { return v[i]; }
    inline VertexInfo &operator[](int i)      { return v[i]; }
};

struct Model
{
    Array<Triangle> triangles;
    Buffer<Color> texture_map;
    Buffer<Color> specular_map;
    Buffer<Vector3> normals_map;
    // useful flags
    bool nm_tangent, flat_shading;

    void release();

    Model() {}
};

struct Object
{
    Vector3 position;
    Model* model;

    Object()         { model = NULL; }
    Object(Model* m) { model = m;    }
};

void normalize(Array<Vector3> &mesh);

void SmoothImage(FrameBuffer &buffer);

void drawLine(Vector3 &v1, Vector3 &v2, Color _c, FrameBuffer& buffer, ZBuffer& zbuffer);

void drawTriangle(Vector3* v, Color _c, FrameBuffer& buffer, ZBuffer& zbuffer);


#endif // graphics_h
