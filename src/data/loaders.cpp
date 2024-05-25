
#include "data/loaders.h"
#include "extern/stb_image.h"
#include "extern/stb_image_write.h"


// algorithm to convert polygonal faces into triangles
void Triangulate(VertexIndex* points, int count, Array<FaceIndex> &dest)
{
    if (count > 4)
    {
        // TODO : apply triangulation algorithm
    }
    else if (count == 3) // triangle, no need for sampling
    {
        FaceIndex f;
        
        f.v[0] = points[0];
        f.v[1] = points[1];
        f.v[2] = points[2];

        dest.add(f);
    }
    else if (count == 4) // quad face
    {
        FaceIndex f1, f2;

        f1.v[0] = points[0];
        f1.v[1] = points[1];
        f1.v[2] = points[2];

        f2.v[0] = points[2];
        f2.v[1] = points[3];
        f2.v[2] = points[0];

        dest.add(f1);
        dest.add(f2);
    }
}

bool loadImageData(const char* filename, FrameBuffer &buffer)
{
    int _w, _h;
    unsigned char* _data = stbi_load(filename, &_w, &_h, NULL, 3);
    if (_data == NULL) return false;

    buffer.release();  // make sure it's empty
    buffer.init(_w, _h);

    // copy and flip vertically
    for (int x = 0; x < buffer.width(); x++)
    {
        for (int y = 0; y < buffer.height(); y++)
        {
            int _idx = x + y * buffer.width();
            int _idx_flip = x + ((buffer.height() - 1) - y)*buffer.width();
            buffer[_idx].r = _data[((_idx_flip) * 3)];
            buffer[_idx].g = _data[((_idx_flip) * 3) + 1];
            buffer[_idx].b = _data[((_idx_flip) * 3) + 2];
        }
    }
    delete[] _data;

    return true;
}


int SaveImageData(const char* filename, FrameBuffer &buffer)
{
    if (buffer.data() == NULL) return 0;
    unsigned char* _data = new unsigned char[buffer.size() * 3];

    // copy and flip vertically
    for (int x = 0; x < buffer.width(); x++)
    {
        for (int y = 0; y < buffer.height(); y++)
        {
            int _idx = x + y * buffer.width();
            int _idx_flip = x + ((buffer.height() - 1) - y)*buffer.width();
            _data[((_idx_flip) * 3)] = buffer[_idx].r;
            _data[((_idx_flip) * 3) + 1] = buffer[_idx].g;
            _data[((_idx_flip) * 3) + 2] = buffer[_idx].b;
        }
    }
    int _result = stbi_write_png(filename, buffer.width(), buffer.height(), 3, (void*)_data, buffer.width() * 3);

    delete[] _data;

    return _result;
}
