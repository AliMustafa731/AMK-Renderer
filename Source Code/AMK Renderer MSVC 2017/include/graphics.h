#ifndef graphics_h
#define graphics_h

#include "geometry.h"
#include <vector>


struct Color
{
    unsigned char b, g, r, a;

    Color(){}
    Color(unsigned char _r, unsigned char _g, unsigned char _b)
    {
        r = _r;
        g = _g;
        b = _b;
    }
};

template<typename T> struct Buffer
{
    size_t width, height, size;
    T *data;

    inline T operator[](size_t i) const { return data[i]; }
    inline T &operator[](size_t i)      { return data[i]; }

    inline T operator()(size_t x, size_t y) const { return data[x + y * width]; }
    inline T &operator()(size_t x, size_t y)      { return data[x + y * width]; }

    void init(int w, int h)
    {
        width = w;
        height = h;
        size = w*h;
        data = new T[size];
    }

    void release()
    {
        if(data != NULL)
        {
            delete[] data;
            data = NULL;
        }
    }

    Buffer()
    {
        width = 0;
        height = 0;
        size = 0;
        data = NULL;
    }

    Buffer(int w, int h) { init(w, h); }
};

typedef Buffer<Color> FrameBuffer;
typedef Buffer<float> ZBuffer;

struct Face
{
    int v[3]; // vertices index
    int t[3]; // uv index
    int n[3]; // vertex normals index

    Face(){}
};

struct Model
{
    std::vector<Vector3> vertices;
    std::vector<Vector3> normals;
    std::vector<Vector2> uv;
    std::vector<Face>    faces;
    Buffer<Color> texture;
    Buffer<Color> specular;
    Buffer<Vector3> normals_map; // normals map
    bool nm_tangent, flat_shading; // useful flags

    Model()
    {
        nm_tangent = false;
        flat_shading = false;
    }

    void release();

    inline Vector3 Vertex(int face_index, int vertex_index)
    {
        return vertices[faces[face_index].v[vertex_index]];
    }
    inline Vector3 Normal(int face_index, int normal_index)
    {
        return normals[faces[face_index].n[normal_index]];
    }
    inline Vector2 UV(int face_index, int uv_index)
    {
        return uv[faces[face_index].t[uv_index]];
    }
};

struct Object
{
    Vector3 position;
    Model* model;

    Object()
    {
        model = NULL;
    }

    Object(Model* m)
    {
        model = m;
    }
};

__forceinline void drawPixel(int x, int y, Color _c, FrameBuffer& buffer)
{
    if (x < buffer.width && x >= 0 && y < buffer.height && y >= 0)
    {
        buffer(x, y) = _c;
    }
}

void normalize(Model *m);

void SmoothImage(FrameBuffer &buffer);

void drawLine(Vector3 &v1, Vector3 &v2, Color _c, FrameBuffer& buffer, ZBuffer& zbuffer);

void drawTriangle(Vector3* v, Color _c, FrameBuffer& buffer, ZBuffer& zbuffer);


#endif // graphics_h
