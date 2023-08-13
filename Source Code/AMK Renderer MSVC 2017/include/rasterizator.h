#ifndef rasterizator_h
#define rasterizator_h

#include "graphics.h"
#include "geometry.h"


extern Vector3 camera;
extern Vector3 camera_offset;
extern Matrix3 camera_matrix;
extern float focal_length;
extern int view_scale;

// control & state flags
extern bool e_render_mode;  // when "false", only render the wire-frame
extern bool e_flat_shading; // enable flat shading
extern bool e_texture;      // enable texture mapping
extern bool e_normals;      // enable normals mapping
extern bool e_specular;     // enable specular mapping
extern bool e_wireframe;    // enable wire-frame

void draw(Object &o, FrameBuffer &buffer, ZBuffer &z_buffer);

void fillTriangle(Object &o, FrameBuffer &buffer, ZBuffer &z_buffer, int face_index);

void lookAt(Vector3 eye, Vector3 center, Vector3 up);

Vector3 projection(Vector3 v);


#endif // rasterizator_h
