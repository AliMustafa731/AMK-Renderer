#include "graphics.h"
#include "common.h"


void normalize(Model *m)
{
    if(m == NULL) return;

    float max_length = 0;

    for(int i = 0 ; i < m->vertices.size() ; i++)
    {
        max_length = _max(max_length, length(m->vertices[i]));
    }
    for(int j = 0 ; j < m->vertices.size() ; j++)
    {
        m->vertices[j].x = m->vertices[j].x / max_length;
        m->vertices[j].y = m->vertices[j].y / max_length;
        m->vertices[j].z = m->vertices[j].z / max_length;
    }
}

void Model::release()
{
    this->texture.release();
    this->specular.release();
    this->normals_map.release();
    this->vertices.clear();
    this->uv.clear();
    this->normals.clear();
    this->faces.clear();
}


void SmoothImage(FrameBuffer &buffer)
{
    if(buffer.data == NULL) return;

    Color* _temp = new Color[buffer.size];

    for(int i = 1 ; i < buffer.width - 1 ; i++) // calculating average colors for smoothing
    {
        for(int j = 1 ; j < buffer.height - 1 ; j++)
        {
            Color center      = buffer[(i) + (j)*buffer.width];
            Color up          = buffer[(i) + (j+1)*buffer.width];
            Color down        = buffer[(i) + (j-1)*buffer.width];
            Color right       = buffer[(i+1) + (j)*buffer.width];
            Color left        = buffer[(i-1) + (j)*buffer.width];
            Color up_left     = buffer[(i-1) + (j+1)*buffer.width];
            Color up_right    = buffer[(i+1) + (j+1)*buffer.width];
            Color down_left   = buffer[(i-1) + (j-1)*buffer.width];
            Color down_right  = buffer[(i+1) + (j-1)*buffer.width];

            int _idx = i + j*buffer.width;
            _temp[_idx].r = center.r/2 + (up.r+down.r+right.r+left.r + up_left.r+up_right.r+down_left.r+down_right.r)/16;
            _temp[_idx].g = center.g/2 + (up.g+down.g+right.g+left.g + up_left.g+up_right.g+down_left.g+down_right.g)/16;
            _temp[_idx].b = center.b/2 + (up.b+down.b+right.b+left.b + up_left.b+up_right.b+down_left.b+down_right.b)/16;
        }
    }
    for(int k = 0 ; k < buffer.size ; k++) // copy the final pixels
    {
        buffer[k] = _temp[k];
    }

    delete _temp;
}


void drawTriangle(Vector3* v, Color _c, FrameBuffer& buffer, ZBuffer& zbuffer)
{
    drawLine(v[0], v[1], _c, buffer, zbuffer);
    drawLine(v[0], v[2], _c, buffer, zbuffer);
    drawLine(v[1], v[2], _c, buffer, zbuffer);
}

void drawLine(Vector3 &v1, Vector3 &v2, Color _c, FrameBuffer& buffer, ZBuffer& zbuffer)
{
    bool steep = false;

    int x1 = _max(0, _min(v1.x, buffer.width-1));
    int y1 = _max(0, _min(v1.y, buffer.height-1));;
    float z1 = v1.z;
    int x2 = _max(0, _min(v2.x, buffer.width-1));
    int y2 = _max(0, _min(v2.y, buffer.height-1));;
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

    for (int _x = x1 ; _x < x2 ; _x++)
    {
        float t = (float)(_x - x1)/(float)(x2 - x1);
        int y = y1 * (1.0f - t) + y2 * t;
        float z = z1 * (1.0f - t) + z2 * t;
        int x = _x;

        if (steep) std::swap(x, y);

        if (z >= zbuffer(x, y) - 0.1f)
        {
            zbuffer(x, y) = z;
            drawPixel(x, y, _c, buffer);
        }
    }
}

void Triangulate(int* points, int num ,std::vector<Face> &dest) // sampling polygonal faces into triangles
{
    if(num == 3) // it's a triangle !!!
    {
        Face f;
        for(int h = 0 ; h < 3 ; h++)
        {
            f.v[h] = points[(h*3)];
            f.t[h] = points[(h*3)+1];
            f.n[h] = points[(h*3)+2];
        }
        dest.push_back(f);
    }
    else if(num == 4) // quad face
    {
        Face f1, f2;

        for(int h = 0 ; h < 3 ; h++)
        {
            f1.v[h] = points[(h*3)];
            f1.t[h] = points[(h*3)+1];
            f1.n[h] = points[(h*3)+2];
        }

        f2.v[0] = points[2*3];  f2.t[0] = points[(2*3)+1];  f2.n[0] = points[(2*3)+2];
        f2.v[1] = points[3*3];  f2.t[1] = points[(3*3)+1];  f2.n[1] = points[(3*3)+2];
        f2.v[2] = points[0];    f2.t[2] = points[1];        f2.n[2] = points[2];

        dest.push_back(f1);
        dest.push_back(f2);
    }
}


