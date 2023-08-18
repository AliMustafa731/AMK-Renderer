
#include "program.h"
#include "system.h"
#include "loaders.h"
#include "stb_image.h"
#include "stb_image_write.h"
#include <iostream>
#include <fstream>
#include <sstream>


struct Vertex
{
    int vert, uv, norm;

    Vertex(){}
};


void Triangulate(Vertex* points, int count, std::vector<Face> &dest) // sampling polygonal faces into triangles
{
    if (count > 4)
    {
        // TODO : apply triangulation algorithm
    }
    else if (count == 3) // triangle, no need for sampling
    {
        Face f;
        for (int h = 0; h < 3; h++)
        {
            f.v[h] = points[h].vert;
            f.t[h] = points[h].uv;
            f.n[h] = points[h].norm;
        }
        dest.push_back(f);
    }
    else if (count == 4) // quad face
    {
        Face f1, f2;

        for (int h = 0; h < 3; h++)
        {
            f1.v[h] = points[h].vert;
            f1.t[h] = points[h].uv;
            f1.n[h] = points[h].norm;
        }

        f2.v[0] = points[2].vert;  f2.t[0] = points[2].uv;  f2.n[0] = points[2].norm;
        f2.v[1] = points[3].vert;  f2.t[1] = points[3].uv;  f2.n[1] = points[3].norm;
        f2.v[2] = points[0].vert;  f2.t[2] = points[0].uv;  f2.n[2] = points[0].norm;

        dest.push_back(f1);
        dest.push_back(f2);
    }
}


void loadFromOBJFile(const char* filename, Model *m)
{
    // open the file and read it into a single string
    std::ifstream file;
    std::string file_contents;

    file.open(filename, std::ifstream::in);

    if (file.fail()) { return; }

    file.seekg(0, std::ios::end);
    int length = file.tellg();
    file.seekg(0, std::ios::beg);

    file_contents.resize(length);
    file.read(&file_contents[0], length);

    file.close();

    std::stringstream s_file(file_contents);
    std::string line;

    // recording the number of elements to later reserve memory for them
    int numVertices = 0;
    int numUV = 0;
    int numNorms = 0;
    int numFaces = 0;

    while (std::getline(s_file, line))
    {
        if (!line.compare(0, 2, "v ")) // vertex
        {
            numVertices += 1;
        }
        else if (!line.compare(0, 3, "vn ")) // normal vector
        {
            numNorms += 1;
        }
        else if (!line.compare(0, 3, "vt ")) // uv
        {
            numUV += 1;
        }
        else if (!line.compare(0, 2, "f ")) // face
        {
            int numNodes = 0;

            char trash;  int x;
            std::istringstream ss(line.c_str());

            ss >> trash;

            while (ss >> x >> trash >> x >> trash >> x)
            {
                numNodes += 1;
            }

            numFaces += numNodes - 2; // (number of triangle faces) = (number of vertices - 2)
        }
    }

    // reserve memory
    m->vertices.reserve(numVertices);
    m->uv.reserve(numUV);
    m->normals.reserve(numNorms);
    m->faces.reserve(numFaces);

    m->nm_tangent = false;
    m->flat_shading = false;
    FrameBuffer normals_texture;
    Vertex points[128]; // temporary storage for points indices

    // processing the file
    s_file.clear();
    s_file.seekg(0, std::ios::beg);

    while (std::getline(s_file, line))
    {
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
            int vert, uv, norm, count = 0;

            while (iss >> vert >> trash >> uv >> trash >> norm)
            {
                // indices start from 1 in .obj format, so subtract 1 from each index
                points[count].vert = vert - 1;
                points[count].uv = uv - 1;
                points[count].norm = norm - 1;
                count++;
            }

            Triangulate(points, count, m->faces);
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

    normalize(m);
    SmoothImage(m->texture);
    SmoothImage(normals_texture);

    // convert texture pixels to Vector3 array
    // change the range from [0, 255] into [-1, 1]
    if (normals_texture.data != NULL)
    {
        m->normals_map.init(normals_texture.width, normals_texture.height);

        for (int i = 0; i < normals_texture.size; i++)
        {
            Vector3 _n;
            _n = Vector3
            (
                (float)normals_texture[i].r / 255.0,
                (float)normals_texture[i].g / 255.0,
                (float)normals_texture[i].b / 255.0
            );
            _n = Vector3((_n.x * 2.0) - 1.0, (_n.y * 2.0) - 1.0, (_n.z * 2.0) - 1.0);
            m->normals_map[i] = normalize(_n);
        }

        normals_texture.release();
    }
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
