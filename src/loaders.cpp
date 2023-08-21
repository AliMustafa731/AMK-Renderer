
#include "program.h"
#include "system.h"
#include "loaders.h"
#include "stb_image.h"
#include "stb_image_write.h"
#include <iostream>
#include <fstream>
#include <sstream>


void Triangulate(VertexInfo* points, int count, Array<Face> &dest) // sampling polygonal faces into triangles
{
    if (count > 4)
    {
        // TODO : apply triangulation algorithm
    }
    else if (count == 3) // triangle, no need for sampling
    {
        Face f;
        
        f.v[0] = points[0];
        f.v[1] = points[1];
        f.v[2] = points[2];

        dest.add(f);
    }
    else if (count == 4) // quad face
    {
        Face f1, f2;

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
    Array<Vector3> vertices(numVertices);
    Array<Vector3> normals(numNorms);
    Array<Vector2> uvs(numUV);
    Array<Face> faces(numFaces);

    m->nm_tangent = false;
    m->flat_shading = false;
    FrameBuffer normals_texture;
    VertexInfo points[128]; // temporary storage for points indices

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
            vertices.add(v);
        }
        else if (!line.compare(0, 3, "vn ")) // normal vector
        {
            iss >> trash >> trash;
            Vector3 n;
            iss >> n.x >> n.y >> n.z;
            normals.add(n);
        }
        else if (!line.compare(0, 3, "vt ")) // uv
        {
            iss >> trash >> trash;
            Vector2 uv;
            iss >> uv.x >> uv.y;
            uvs.add(uv);
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

            Triangulate(points, count, faces);
        }
        else if (!line.compare(0, 8, "texture ")) // path to texture file
        {
            std::string a;
            iss >> a >> a;
            if (!loadImageData(a.c_str(), m->texture_map))
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
            if (!loadImageData(a.c_str(), m->specular_map))
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

    normalize(vertices);
    SmoothImage(m->texture_map);
    SmoothImage(normals_texture);

    m->triangles.init(faces.counter);

    for (int i = 0; i < m->triangles.size; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            m->triangles[i].vert[j] = vertices[faces[i].v[j].vert];
            m->triangles[i].uv[j] = uvs[faces[i].v[j].uv];
            m->triangles[i].norm[j] = normals[faces[i].v[j].norm];
        }
    }

    vertices.release();
    normals.release();
    uvs.release();
    faces.release();

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
