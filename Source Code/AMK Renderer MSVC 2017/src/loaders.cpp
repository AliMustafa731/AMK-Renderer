
#include "program.h"
#include "system.h"
#include "loaders.h"
#include "stb_image.h"
#include "stb_image_write.h"
#include <iostream>
#include <fstream>
#include <sstream>


void loadFromOBJFile(const char* filename, Model *m)
{
    std::ifstream file;
    file.open(filename, std::ifstream::in);

    if (file.fail())
    {
        return;
    }

    m->nm_tangent = false;
    m->flat_shading = false;
    FrameBuffer normals_texture;
    std::string line;
    int points[128 * 3]; // temporary storage for points indices

    while (!file.eof()) // loading and processing the file
    {
        std::getline(file, line);
        std::istringstream iss(line.c_str());
        char trash;

        if (!line.compare(0, 2, "v ")) // vertex
        {
            iss >> trash;
            Vector3 v;
            iss >> v.x >> v.y >> v.z;
            m->vertices.push_back(v);
        }
        else if (!line.compare(0, 3, "vn ")) // normal vector
        {
            iss >> trash >> trash;
            Vector3 n;
            iss >> n.x >> n.y >> n.z;
            m->normals.push_back(n);
        }
        else if (!line.compare(0, 3, "vt ")) // uv
        {
            iss >> trash >> trash;
            Vector2 uv;
            iss >> uv.x >> uv.y;
            m->uv.push_back(uv);
        }
        else if (!line.compare(0, 2, "f ")) // face
        {
            iss >> trash;
            int v, t, n, k = 0;

            while (iss >> v >> trash >> t >> trash >> n)
            {
                // indices start from 1 in .obj format, so subtract 1 from each index
                points[(k * 3)] = v - 1;
                points[(k * 3) + 1] = t - 1;
                points[(k * 3) + 2] = n - 1;
                k++;
            }

            Triangulate(points, k, m->faces);
        }
        else if (!line.compare(0, 8, "texture ")) // path to texture file
        {
            std::string a;
            iss >> a >> a;
            if (!loadImageData(a.c_str(), m->texture))
            {
                std::string msg = "can't load texture map \"" + a + "\"";
                MessageBox(main_program->win_handle, msg.c_str(), "Error", MB_OK);
            }
        }
        else if (!line.compare(0, 7, "normal ")) // path to normal texture file
        {
            std::string a;
            iss >> a >> a;
            if (!loadImageData(a.c_str(), normals_texture))
            {
                std::string msg = "can't load normal map \"" + a + "\"";
                MessageBox(main_program->win_handle, msg.c_str(), "Error", MB_OK);
            }
        }
        else if (!line.compare(0, 9, "specular "))// path to specular texture file
        {
            std::string a;
            iss >> a >> a;
            if (!loadImageData(a.c_str(), m->specular))
            {
                std::string msg = "can't load specular map \"" + a + "\"";
                MessageBox(main_program->win_handle, msg.c_str(), "Error", MB_OK);
            }
        }
        else if (!line.compare(0, 16, "nm_tangent_space")) // set a flag
        {
            m->nm_tangent = true;
        }
        else if (!line.compare(0, 12, "flat_shading")) // set a flag
        {
            m->flat_shading = true;
        }
    }
    file.close();

    normalize(m);
    SmoothImage(m->texture);
    SmoothImage(normals_texture);

    // convert texture pixels to Vector3 array
    // convert vector coordinates range from [0, 255] into [-1, 1]
    if (normals_texture.data != NULL)
    {
        m->normals_map.init(normals_texture.width, normals_texture.height);

        for (int i = 0; i < normals_texture.size; i++)
        {
            Vector3 _n;
            _n = Vector3((float)normals_texture[i].r / 255.0,
                (float)normals_texture[i].g / 255.0,
                (float)normals_texture[i].b / 255.0);
            _n = Vector3((_n.x * 2.0) - 1.0, (_n.y * 2.0) - 1.0, (_n.z * 2.0) - 1.0);
            m->normals_map[i] = normalize(_n);
        }

        normals_texture.release();
    }

    // Enabling & Disabling controls depending on which textures the model has
    EnableWindow(GetDlgItem(main_program->win_handle, ID_TEXTURE_MAPPING), (m->texture.data != NULL));
    EnableWindow(GetDlgItem(main_program->win_handle, ID_NORMAL_MAPPING), (m->normals_map.data != NULL));
    EnableWindow(GetDlgItem(main_program->win_handle, ID_SPECULAR_MAPPING), (m->specular.data != NULL));
    EnableWindow(GetDlgItem(main_program->win_handle, ID_FLAT_SHADING), (!m->flat_shading));
}

int loadImageData(const char* filename, FrameBuffer &buffer)
{
    int _w, _h;
    unsigned char* _data = stbi_load(filename, &_w, &_h, NULL, 3);
    if (_data == NULL) return 0;

    buffer.release();  // make sure it's empty
    buffer.init(_w, _h);

    // copy and flip vertically
    for (int x = 0; x < buffer.width; x++)
    {
        for (int y = 0; y < buffer.height; y++)
        {
            int _idx = x + y * buffer.width;
            int _idx_flip = x + ((buffer.height - 1) - y)*buffer.width;
            buffer[_idx].r = _data[((_idx_flip) * 3)];
            buffer[_idx].g = _data[((_idx_flip) * 3) + 1];
            buffer[_idx].b = _data[((_idx_flip) * 3) + 2];
        }
    }
    delete _data;

    return 1;
}

int SaveImageData(const char* filename, FrameBuffer &buffer)
{
    if (buffer.data == NULL) return 0;
    unsigned char* _data = new unsigned char[buffer.size * 3];

    // copy and flip vertically
    for (int x = 0; x < buffer.width; x++)
    {
        for (int y = 0; y < buffer.height; y++)
        {
            int _idx = x + y * buffer.width;
            int _idx_flip = x + ((buffer.height - 1) - y)*buffer.width;
            _data[((_idx_flip) * 3)] = buffer[_idx].r;
            _data[((_idx_flip) * 3) + 1] = buffer[_idx].g;
            _data[((_idx_flip) * 3) + 2] = buffer[_idx].b;
        }
    }
    int _result = stbi_write_png(filename, buffer.width, buffer.height, 3, (void*)_data, buffer.width * 3);

    delete _data;

    return _result;
}
