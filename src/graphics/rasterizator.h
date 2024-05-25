#ifndef rasterizator_h
#define rasterizator_h

#include "graphics/graphics.h"
#include "geometry/geometry.h"

//
// Structure holding 3D camera information
//
struct Camera
{
    Vector3 camera_offset;
    Matrix3 camera_matrix;
    float focal_length = 5.0f;

    Camera(){}

    //
    // Perspective project a point depending on the camera (Orientation / Position)
    //
    Vector3 project(Vector3 v);

    //
    // change the (Orientation / Position) of the camera
    //
    void lookAt(Vector3 eye, Vector3 center, Vector3 up);
};

//
// Structure to store rendering state flags
// used by the GUI to determine which controls to (Enable/Disable)
//
struct RenderState
{
    RenderState(){}

    bool e_render_mode;  // when "false", only render the wire-frame
    bool e_flat_shading; // enable flat shading
    bool e_texture;      // enable texture mapping
    bool e_normals;      // enable normals mapping
    bool e_specular;     // enable specular mapping
    bool e_wireframe;    // enable wire-frame

    bool _texture_mapping;
    bool _normal_mapping;
    bool _specular_mapping;
    bool _flat_shading;
};

//
// Class containing rendering functions
//
class Rasterizor
{
public:

    Rasterizor() : light_src(0.4f, -0.25f, 5.0f)
    {}

    //
    // Draw a line
    //
    void drawLine(Vector3 &v1, Vector3 &v2, Color _c, FrameBuffer& buffer, ZBuffer& zbuffer);

    //
    // Draw a non-filled Triangle
    //
    void drawTriangle(Vector3* v, Color _c, FrameBuffer& buffer, ZBuffer& zbuffer);

    //
    // A Function used to fill a (Transformed/Projected) "Face"
    //
    void fillTriangle(Face& face, Object &o, Camera& camera, FrameBuffer &buffer, ZBuffer &z_buffer, RenderState& render_state);

    //
    // A Function that takes a single "Face" then : Transform, Project, Scale it to screen
    // return "true" to ignore (Don't render) the "Face"
    //
    bool VertexShader(Face& face, Object &o, Camera& camera, FrameBuffer &buffer, ZBuffer& zbuffer, RenderState& render_state);

    //
    // A Function that's called at every pixel of a (Transformed/Projected) "Face"
    // It serves to determine the color of the current pixel,
    // depending on : (Textures / Normal Maps / Lightining, etc)
    //
    Color FragmentShader(Face& face, Object &o, Camera& camera, Vector3 &bc_world, RenderState& render_state);

    //
    // Render a whole 3D object
    //
    void draw(Object &o, Camera& camera, FrameBuffer &buffer, ZBuffer &z_buffer, RenderState& render_state);

private:

    // viewport
    Vector3 screen_offset;
    int view_scale = 100;

    //---------------------------------------------------------------
    //   Variables used internally by rendering pipline
    //   functions : (VertexShader) and (FragmentShader)
    //---------------------------------------------------------------

    // a matrix that's used for (Tangent space normals mapping)
    Matrix3 tangent_basis;

    Vector3 light_src;
    Vector3 tri_projected[3];
    Vector3 tri_screen[3];
    Vector3 n;    // normal vector
    Vector2 uv;   // texture coordinates
    float _w[3];
};

#endif // rasterizator_h
