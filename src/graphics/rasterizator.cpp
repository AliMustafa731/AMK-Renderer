
#include "graphics/rasterizator.h"
#include "program/program.h"
#include "common.h"

#include <cmath>
#include <utility>

//
// Render a whole 3D object
//
void Rasterizor::draw(Object &o, Camera& camera, FrameBuffer &buffer, ZBuffer &z_buffer, RenderState& render_state)
{
    screen_offset.x = buffer.width() / 2;
    screen_offset.y = buffer.height() / 2;

    render_state._texture_mapping = (render_state.e_texture && o.model->texture_map.data() != NULL);
    render_state._normal_mapping = (render_state.e_normals && o.model->normals_map.data() != NULL);
    render_state._specular_mapping = (render_state.e_specular && o.model->specular_map.data() != NULL);
    render_state._flat_shading = (render_state.e_flat_shading || o.model->flat_shading);

    for (int i = 0; i < o.model->mesh.size(); i++)
    {
        Face face = o.model->mesh[i];

        if (Rasterizor::VertexShader(face, o, camera, buffer, z_buffer, render_state))
        {
            continue;  // ignore this face, (e.g. it's facing backward)
        }

        Rasterizor::fillTriangle(face, o, camera, buffer, z_buffer, render_state);

        if (render_state.e_wireframe)  // only render wireframe
        {
            Rasterizor::drawTriangle(tri_screen[0], tri_screen[1], tri_screen[2], Color(255, 255, 255), buffer, z_buffer);
        }
    }
}

//
// A Function that takes a single "Face" then : Transform, Project, Scale it to screen
// return "true" to ignore (Don't render) the "Face"
//
bool Rasterizor::VertexShader(Face& face, Object &o, Camera& camera, FrameBuffer &buffer, ZBuffer& zbuffer, RenderState& render_state)
{
    // transformation and perspective projection
    for (int k = 0; k < 3; k++)
    {
        face[k].vert = transform(face[k].vert, camera.camera_matrix);
        face[k].vert = add(face[k].vert, o.position);
        face[k].vert = sub(face[k].vert, camera.camera_offset);

        tri_projected[k] = camera.project(face[k].vert);
    }

    // back-face removal
    n = normalize(crossProduct(sub(tri_projected[2], tri_projected[0]), sub(tri_projected[1], tri_projected[0])));

    if (n.z < 0 && render_state.e_render_mode)
    {
        return true; // the triangle is facing backward, ignore it
    }

    n = normalize(crossProduct(sub(face[2].vert, face[0].vert), sub(face[1].vert, face[0].vert)));

    // translating 3D points to the screen
    for (int h = 0; h < 3; h++)
    {
        tri_screen[h] = tri_projected[h];
        tri_screen[h].x = (tri_projected[h].x * view_scale) + screen_offset.x;
        tri_screen[h].y = (tri_projected[h].y * view_scale) + screen_offset.y;

        if (render_state._texture_mapping || render_state._normal_mapping || render_state._specular_mapping)
        {
            face[h].uv.x = _max(0.0f, _min(1.0f, face[h].uv.x));
            face[h].uv.y = _max(0.0f, _min(1.0f, face[h].uv.y));
        }

        if (!render_state._flat_shading)
        {
            face[h].norm = normalize(transform(face[h].norm, camera.camera_matrix));
        }

        // optimization : computing tangent basis for "flat shading"
        if (render_state._flat_shading && render_state._normal_mapping && o.model->nm_tangent)
        {
            tangent_basis = TangentBasis(face, n);
        }
    }

    if (!render_state.e_render_mode) // only render the wire-frame
    {
        drawTriangle(tri_screen[0], tri_screen[1], tri_screen[2], Color(255, 255, 255), buffer, zbuffer);
        return true;
    }

    return false;
}

//
// A Function used to fill a (Transformed/Projected) "Face"
//
void Rasterizor::fillTriangle(Face& face, Object &o, Camera& camera, FrameBuffer &buffer, ZBuffer &z_buffer, RenderState& render_state)
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
            bc_screen = barycentric(tri_screen[0], tri_screen[1], tri_screen[2], p);

            if (bc_screen.x <= 0 || bc_screen.y <= 0 || bc_screen.z <= 0)
            {
                continue;  // pixel is outside the triangle, ignore it
            }

            // apply perspective deformation
            // divide by the 4th component
            bc_world = Vector3(bc_screen.x / tri_projected[0].w, bc_screen.y / tri_projected[1].w, bc_screen.z / tri_projected[2].w);

            // normalize, divide by the sum
            bc_world = div(bc_world, (bc_world.x + bc_world.y + bc_world.z));

            float _z = tri_screen[0].z*bc_screen.x + tri_screen[1].z*bc_screen.y + tri_screen[2].z*bc_screen.z;

            if (_z > z_buffer(p.x, p.y))
            {
                z_buffer(p.x, p.y) = _z;
                Color c = FragmentShader(face, o, camera, bc_world, render_state);
                buffer(p.x, p.y) = c;  // draw the pixel finally
            }
        }
    }
}

//
// A Function that's called at every pixel of a (Transformed/Projected) "Face"
// It serves to determine the color of the current pixel,
// depending on : (Textures / Normal Maps / Lightining, etc)
//
Color Rasterizor::FragmentShader(Face& face, Object &o, Camera& camera, Vector3 &bc_world, RenderState& render_state)
{
    Color c(255, 255, 255);
    float intensity = 0, spec = 0, spec_intensity = 0;

    if (!render_state._flat_shading) // interpolated normal vectors
    {
        n.x = face[0].norm.x*bc_world.x + face[1].norm.x*bc_world.y + face[2].norm.x*bc_world.z;
        n.y = face[0].norm.y*bc_world.x + face[1].norm.y*bc_world.y + face[2].norm.y*bc_world.z;
        n.z = face[0].norm.z*bc_world.x + face[1].norm.z*bc_world.y + face[2].norm.z*bc_world.z;
        n = normalize(n);
    }

    if (render_state._texture_mapping || render_state._normal_mapping || render_state._specular_mapping) // uv mapping
    {
        uv.x = face[0].uv.x*bc_world.x + face[1].uv.x*bc_world.y + face[2].uv.x*bc_world.z;
        uv.y = face[0].uv.y*bc_world.x + face[1].uv.y*bc_world.y + face[2].uv.y*bc_world.z;
    }

    if (render_state._texture_mapping) // texture mapping
    {
        int _x = uv.x*(o.model->texture_map.width() - 1);
        int _y = uv.y*(o.model->texture_map.height() - 1);

        c = o.model->texture_map(_x, _y);
    }

    if (render_state._normal_mapping) // normals mapping
    {
        int _x = uv.x*(o.model->normals_map.width() - 1);
        int _y = uv.y*(o.model->normals_map.height() - 1);

        if (o.model->nm_tangent) // tangent space normal mapping
        {
            if (!render_state._flat_shading) { tangent_basis = TangentBasis(face, n); }

            n = normalize(transform(o.model->normals_map(_x, _y), tangent_basis));
        }
        else // object space normal mapping
        {
            n = normalize(transform(o.model->normals_map(_x, _y), camera.camera_matrix));
        }
    }

    // current pixel in 3D space
    Vector3 point = Vector3(
        face[0].vert.x*bc_world.x + face[1].vert.x*bc_world.y + face[2].vert.x*bc_world.z,
        face[0].vert.y*bc_world.x + face[1].vert.y*bc_world.y + face[2].vert.y*bc_world.z,
        face[0].vert.z*bc_world.x + face[1].vert.z*bc_world.y + face[2].vert.z*bc_world.z
    );

    Vector3 l = normalize(sub(light_src, point)); // The vector from current pixel's point to the light source
    intensity = _max(0.2f, dotProduct(n, l));  // clipp intensity at 0.2

    if (render_state._specular_mapping) // specular mapping
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

//
// Draw a non-filled Triangle
//
void Rasterizor::drawTriangle(Vector3 v1, Vector3 v2, Vector3 v3, Color _c, FrameBuffer& buffer, ZBuffer& zbuffer)
{
    drawLine(v1, v2, _c, buffer, zbuffer);
    drawLine(v1, v3, _c, buffer, zbuffer);
    drawLine(v2, v3, _c, buffer, zbuffer);
}
 
//
// Draw a line
//
void Rasterizor::drawLine(Vector3 &v1, Vector3 &v2, Color _c, FrameBuffer& buffer, ZBuffer& zbuffer)
{
    bool steep = false;

    int x1 = _max(0, _min(v1.x, buffer.width() - 1));
    int y1 = _max(0, _min(v1.y, buffer.height() - 1));;
    float z1 = v1.z;
    int x2 = _max(0, _min(v2.x, buffer.width() - 1));
    int y2 = _max(0, _min(v2.y, buffer.height() - 1));;
    float z2 = v2.z;

    int dx = std::abs(x1 - x2);
    int dy = std::abs(y1 - y2);

    if (dx < dy)
    {
        std::swap(x1, y1);
        std::swap(x2, y2);
        steep = true;
    }
    if (x1 > x2) // make it left to right
    {
        std::swap(x1, x2);
        std::swap(y1, y2);
        std::swap(z1, z2);
    }

    for (int _x = x1; _x < x2; _x++)
    {
        float t = (float)(_x - x1) / (float)(x2 - x1);
        int y = y1 * (1.0f - t) + y2 * t;
        float z = z1 * (1.0f - t) + z2 * t;
        int x = _x;

        if (steep) std::swap(x, y);

        if (z >= zbuffer(x, y) - 0.2f)
        {
            zbuffer(x, y) = z;
            buffer(x, y) = _c;
        }
    }
}

//
// change the (Orientation / Position) of the camera
//
void Camera::lookAt(Vector3 eye, Vector3 center, Vector3 up)
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

//
// Perspective project a point depending on the camera (Orientation / Position)
//
Vector4 Camera::project(Vector3 v)
{
    Vector4 _v;

    if (focal_length != 0.0f)
    {
        _v.w = 1.0f - (v.z / focal_length);

        if (_v.w != 0.0f)
        {
            // apply perspective deformation
            // divide by the 4th component
            _v = Vector4(v.x / _v.w, v.y / _v.w, v.z / _v.w, _v.w);
        }
    }

    return _v;
}

