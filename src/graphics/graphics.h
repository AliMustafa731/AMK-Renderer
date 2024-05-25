#ifndef graphics_h
#define graphics_h

#include "geometry/geometry.h"
#include "data/array.h"

#include <cstdint>

// RGBA 32-bit Color
struct Color
{
    uint8_t b, g, r, a;

    Color(){}

    Color(uint8_t _r, uint8_t _g, uint8_t _b) : r(_r), g(_g), b(_b), a(255) {}
};

typedef Buffer<Color> FrameBuffer;
typedef Buffer<float> ZBuffer;

// Structure holding information of a single point in 3D space
struct Vertex
{
    Vector3 vert;  // vertex position in 3D space
    Vector2 uv;    // texture coordinates in 2D space (used to map 2D textures into 3D faces)
    Vector3 norm;  // normal vector (used for lightining)

    Vertex(){}
    
    Vertex(Vector3 _vert, Vector2 _uv, Vector3 _norm) : vert(_vert), uv(_uv), norm(_norm) {}
};

// Structure holding information of a (3) point in 3D space
struct Face
{
    Vertex v[3];

    Face() {}

    Face(Vertex v1, Vertex v2, Vertex v3)
    {
        v[0] = v1;
        v[1] = v2;
        v[2] = v3;
    }

    inline Vertex operator[](size_t i) const { return v[i]; }
    inline Vertex &operator[](size_t i) { return v[i]; }
};

//
// Structure holding the indices of single point
// these indices are used to look-up elements from an array
// which is the usual method used to store 3D object's information
//
struct VertexIndex
{
    uint32_t vert;  // vertex index
    uint32_t uv;    // texture index
    uint32_t norm;  // normal index

    VertexIndex() {}

    VertexIndex(uint32_t _vert, uint32_t _uv, uint32_t _norm) : vert(_vert), uv(_uv), norm(_norm) {}
};

//
// Structure holding the indices of (3) points.
// these indices are used to look-up elements from an array,
// which is the usual method used to store 3D object's information
//
struct FaceIndex
{
    VertexIndex v[3];

    FaceIndex() {}

    FaceIndex(VertexIndex v1, VertexIndex v2, VertexIndex v3)
    {
        v[0] = v1;
        v[1] = v2;
        v[2] = v3;
    }

    inline VertexIndex operator[](size_t i) const { return v[i]; }
    inline VertexIndex &operator[](size_t i) { return v[i]; }
};

//
// Structure holding the entire structure of a 3D model
//
struct Model
{
    Array<Face> mesh;
    Buffer<Color> texture_map;
    Buffer<Color> specular_map;
    Buffer<Vector3> normals_map;

    // useful flags
    bool nm_tangent, flat_shading;

    bool loadFromOBJFile(const char* filename);
    void release();

    Model() {}
};

//
// Structure holding a 3D object entity
// with attributes such as :
//   - Position
//   - Transformations (rotation, scaling and translation)
//
// multiple "Objects" may use the same "Model"
//
struct Object
{
    Vector3 position;
    Model* model;

    Object() : model(NULL) {}
    Object(Model* m) : model(m) {}
};


//
// static class that contains utility functions
//
class Util
{
public:

    // normalize an array of 3D Vectors by the longest magnitude
    static void normalize(Array<Vector3> &mesh);

    // smmoth an image by taking the average of each 3x3 block
    static void smoothImage(FrameBuffer &buffer);
};

#endif // graphics_h
