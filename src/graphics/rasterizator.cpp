
#include "graphics/rasterizator.h"
#include "program/program.h"
#include "common.h"
#include <cmath>


Vector3 camera_offset;
Matrix3 camera_matrix;
Vector3 screen_offset;
Matrix3 tangent_basis;

int view_scale = 100;
float focal_length = 5.0f;

// control & state flags
bool e_render_mode = true;   // when "false", only render the wire-frame
bool e_flat_shading = false; // enable flat shading
bool e_texture = true;       // enable texture mapping
bool e_normals = true;       // enable normals mapping
bool e_specular = true;      // enable specular mapping
bool e_wireframe = false;    // enable wire-frame

bool _texture_mapping;
bool _normal_mapping;
bool _specular_mapping;
bool _flat_shading;

Vector3 light_src(0.4f, -0.25f, 5.0f);
Triangle tri;
Vector3 tri_projected[3], tri_screen[3];
Vector3 n;
Vector2 uv;
float _w[3];

Color white(255, 255, 255);

__forceinline Vector3 projection(Vector3 v)
{
    Vector3 _v;
    float _w = 1.0;

    if (focal_length != 0) { _w = 1.0 - (v.z / focal_length); }

    if (_w != 0.0) { _v = div(v, _w); }

    return _v;
}

// return "true" to ignore current face
__forceinline bool VertexShader(Object &o, FrameBuffer &buffer, ZBuffer& zbuffer)
{
    // transformation and perspective projection
    for (int k = 0; k < 3; k++)
    {
        tri.vert[k] = transform(tri.vert[k], camera_matrix);
        tri.vert[k] = add(tri.vert[k], o.position);
        tri.vert[k] = sub(tri.vert[k], camera_offset);

        tri_projected[k] = projection(tri.vert[k]);
        _w[k] = 1.0 - (tri.vert[k].z / focal_length);
    }

    // back-face removal
    n = normalize(crossProduct(sub(tri_projected[2], tri_projected[0]), sub(tri_projected[1], tri_projected[0])));

    if (n.z < 0 && e_render_mode) return true; // the triangle is facing backward, ignore it

    n = normalize(crossProduct(sub(tri.vert[2], tri.vert[0]), sub(tri.vert[1], tri.vert[0])));

    // translating 3D points to the screen
    for (int h = 0; h < 3; h++)
    {
        tri_screen[h].x = (tri_projected[h].x * view_scale) + screen_offset.x;
        tri_screen[h].y = (tri_projected[h].y * view_scale) + screen_offset.y;
        tri_screen[h].z = tri_projected[h].z;

        if (_texture_mapping || _normal_mapping || _specular_mapping)
        {
            tri.uv[h].x = _max(0.0f, _min(1.0f, tri.uv[h].x));
            tri.uv[h].y = _max(0.0f, _min(1.0f, tri.uv[h].y));
        }

        if (!_flat_shading)
        {
            tri.norm[h] = normalize(transform(tri.norm[h], camera_matrix));
        }

        // optimization : computing tangent basis for "flat shading"
        if (_flat_shading && _normal_mapping && o.model->nm_tangent)
        {
            tangent_basis = TangentBasis(tri.vert, tri.uv, n);
        }
    }

    if (!e_render_mode) // only render the wire-frame
    {
        drawTriangle(tri_screen, white, buffer, zbuffer);
        return true;
    }

    return false;
}


__forceinline Color FragmentShader(Object &o, Vector3 &bc_world)
{
    Color c(255, 255, 255);
    float intensity = 0, spec = 0, spec_intensity = 0;

    if (!_flat_shading) // interpolated normal vectors
    {
        n.x = tri.norm[0].x*bc_world.x + tri.norm[1].x*bc_world.y + tri.norm[2].x*bc_world.z;
        n.y = tri.norm[0].y*bc_world.x + tri.norm[1].y*bc_world.y + tri.norm[2].y*bc_world.z;
        n.z = tri.norm[0].z*bc_world.x + tri.norm[1].z*bc_world.y + tri.norm[2].z*bc_world.z;
        n = normalize(n);
    }

    if (_texture_mapping || _normal_mapping || _specular_mapping) // uv mapping
    {
        uv.x = tri.uv[0].x*bc_world.x + tri.uv[1].x*bc_world.y + tri.uv[2].x*bc_world.z;
        uv.y = tri.uv[0].y*bc_world.x + tri.uv[1].y*bc_world.y + tri.uv[2].y*bc_world.z;
    }

    if (_texture_mapping) // texture mapping
    {
        int _x = uv.x*(o.model->texture_map.width() - 1);
        int _y = uv.y*(o.model->texture_map.height() - 1);

        c = o.model->texture_map(_x, _y);
    }

    if (_normal_mapping) // normals mapping
    {
        int _x = uv.x*(o.model->normals_map.width() - 1);
        int _y = uv.y*(o.model->normals_map.height() - 1);

        if (o.model->nm_tangent) // tangent space normal mapping
        {
            if (!_flat_shading) { tangent_basis = TangentBasis(tri.vert, tri.uv, n); }

            n = normalize(transform(o.model->normals_map(_x, _y), tangent_basis));
        }
        else // object space normal mapping
        {
            n = normalize(transform(o.model->normals_map(_x, _y), camera_matrix));
        }
    }

    // current pixel in 3D space
    Vector3 point = Vector3(
        tri.vert[0].x*bc_world.x + tri.vert[1].x*bc_world.y + tri.vert[2].x*bc_world.z,
        tri.vert[0].y*bc_world.x + tri.vert[1].y*bc_world.y + tri.vert[2].y*bc_world.z,
        tri.vert[0].z*bc_world.x + tri.vert[1].z*bc_world.y + tri.vert[2].z*bc_world.z
    );

    Vector3 l = normalize(sub(light_src, point)); // The vector from current pixel's point to the light source
    intensity = _max(0.2f, dotProduct(n, l));  // clipp intensity at 0.2

    if (_specular_mapping) // specular mapping
    {
        Vector3 r = normalize(sub(mul(n, dotProduct(n, l) * 2), l)); // reflected light

        int _x = uv.x*(o.model->specular_map.width() - 1);
        int _y = uv.y*(o.model->specular_map.height() - 1);

        spec_intensity = (float)o.model->specular_map(_x, _y).r / 127.0f;

        spec = pow(_max(0, r.z), 10);
    }

    c.r = _min(c.r*(intensity + spec_intensity * spec), 255);
    c.g = _min(c.g*(intensity + spec_intensity * spec), 255);
    c.b = _min(c.b*(intensity + spec_intensity * spec), 255);

    return c;
}


void draw(Object &o, FrameBuffer &buffer, ZBuffer &z_buffer)
{
    screen_offset.x = buffer.width() / 2;
    screen_offset.y = buffer.height() / 2;

    _texture_mapping = (e_texture && o.model->texture_map.data() != NULL);
    _normal_mapping = (e_normals && o.model->normals_map.data() != NULL);
    _specular_mapping = (e_specular && o.model->specular_map.data() != NULL);
    _flat_shading = (e_flat_shading || o.model->flat_shading);

    for (int i = 0; i < o.model->triangles.size(); i++)
    {
        tri = o.model->triangles[i];

        if (VertexShader(o, buffer, z_buffer)) { continue; } // ignore this triangle, (e.g. it's facing backward)

        fillTriangle(o, buffer, z_buffer);

        if (e_wireframe) { drawTriangle(tri_screen, white, buffer, z_buffer); }
    }
}


void fillTriangle(Object &o, FrameBuffer &buffer, ZBuffer &z_buffer)
{
    // calculating the rectangle to draw pixels within
    int rect[4] = { buffer.width() - 1, buffer.height() - 1, 0, 0 };

    for (int k = 0; k < 3; k++)
    {
        rect[0] = _max(0, _min(rect[0], tri_screen[k].x));
        rect[1] = _max(0, _min(rect[1], tri_screen[k].y));
        rect[2] = _min(buffer.width() - 1, _max(rect[2], tri_screen[k].x));
        rect[3] = _min(buffer.height() - 1, _max(rect[3], tri_screen[k].y));
    }

    // filling the rectangle with pixels
    Vector3 bc_screen, bc_world;
    Vector2i p;

    for (p.x = rect[0]; p.x <= rect[2]; p.x++)
    {
        for (p.y = rect[1]; p.y <= rect[3]; p.y++)
        {
            bc_screen = barycentric(tri_screen, p);

            if (bc_screen.x <= 0 || bc_screen.y <= 0 || bc_screen.z <= 0)
            {
                continue;  // pixel is outside the triangle, ignore it
            }

            bc_world = Vector3(bc_screen.x / _w[0], bc_screen.y / _w[1], bc_screen.z / _w[2]); // apply perspective deformation
            bc_world = div(bc_world, (bc_world.x + bc_world.y + bc_world.z)); // normalize, divide by the sum

            float _z = tri_screen[0].z*bc_screen.x + tri_screen[1].z*bc_screen.y + tri_screen[2].z*bc_screen.z;

            if (_z > z_buffer(p.x, p.y))
            {
                z_buffer(p.x, p.y) = _z;
                Color c = FragmentShader(o, bc_world);
                buffer(p.x, p.y) = c;  // draw the pixel finally
            }
        }
    }
}


void lookAt(Vector3 eye, Vector3 center, Vector3 up)
{
    eye.x = -eye.x;
    eye.y = -eye.y;

    Vector3 x, y, z; // camera basis unit vectors

    z = normalize(sub(eye, center));
    x = normalize(crossProduct(z, up));
    y = normalize(crossProduct(x, z));

    camera_offset.x = dotProduct(x, eye);
    camera_offset.y = dotProduct(y, eye);
    camera_offset.z = dotProduct(z, eye) - focal_length;

    camera_matrix.set_col(0, x);
    camera_matrix.set_col(1, y);
    camera_matrix.set_col(2, z);
}

