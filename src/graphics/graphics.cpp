
#include "graphics/graphics.h"
#include "program/program.h"
#include "data/loaders.h"
#include "common.h"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>

//
// load 3D model from (.obj) file
//
bool Model::loadFromOBJFile(const char* filename)
{
    // open the file and read it into a single string
    std::ifstream file;
    std::string file_contents;

    file.open(filename, std::ifstream::in);

    if (file.fail()) { return false; }

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

    // reserve temporary memory
    Array<Vector3f>  vertices;      vertices.reserve(numVertices);
    Array<Vector3f>  normals;       normals.reserve(numNorms);
    Array<Vector2f>  uvs;           uvs.reserve(numUV);
    Array<FaceIndex> facesIndex;   facesIndex.reserve(numFaces);

    this->nm_tangent = false;
    this->flat_shading = false;
    FrameBuffer normals_texture;
    VertexIndex points[128]; // temporary storage for points indices

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
            Vector3f v;
            iss >> v.x >> v.y >> v.z;
            vertices.add(v);
        }
        else if (!line.compare(0, 3, "vn ")) // normal vector
        {
            iss >> trash >> trash;
            Vector3f n;
            iss >> n.x >> n.y >> n.z;
            normals.add(n);
        }
        else if (!line.compare(0, 3, "vt ")) // uv
        {
            iss >> trash >> trash;
            Vector2f uv;
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

            Triangulate(points, count, facesIndex);
        }
        else if (!line.compare(0, 8, "texture ")) // path to texture file
        {
            std::string a;
            iss >> a >> a;
            if (!loadImageData(a.c_str(), this->texture_map))
            {
                std::string msg = "can't load texture map \"" + a + "\"";
                MessageBox(Program::main_program->WindowHandle(), msg.c_str(), "Error", MB_OK);
            }
        }
        else if (!line.compare(0, 7, "normal ")) // path to normal texture file
        {
            std::string a;
            iss >> a >> a;
            if (!loadImageData(a.c_str(), normals_texture))
            {
                std::string msg = "can't load normal map \"" + a + "\"";
                MessageBox(Program::main_program->WindowHandle(), msg.c_str(), "Error", MB_OK);
            }
        }
        else if (!line.compare(0, 9, "specular "))// path to specular texture file
        {
            std::string a;
            iss >> a >> a;
            if (!loadImageData(a.c_str(), this->specular_map))
            {
                std::string msg = "can't load specular map \"" + a + "\"";
                MessageBox(Program::main_program->WindowHandle(), msg.c_str(), "Error", MB_OK);
            }
        }
        else if (!line.compare(0, 16, "nm_tangent_space")) // set a flag
        {
            this->nm_tangent = true;
        }
        else if (!line.compare(0, 12, "flat_shading")) // set a flag
        {
            this->flat_shading = true;
        }
    }

    Util::normalize(vertices);
    Util::smoothImage(this->texture_map);
    Util::smoothImage(normals_texture);

    // resolve the indices of (vertex, uv, normal) to form faces
    this->mesh.init(facesIndex.size());

    for (int i = 0; i < this->mesh.size(); i++)
    {
        for (int j = 0; j < 3; j++)
        {
            this->mesh[i].v[j].vert = vertices[facesIndex[i].v[j].vert];
            this->mesh[i].v[j].uv = uvs[facesIndex[i].v[j].uv];
            this->mesh[i].v[j].norm = normals[facesIndex[i].v[j].norm];
        }
    }

    vertices.release();
    normals.release();
    uvs.release();
    facesIndex.release();

    // convert texture pixels to Vector3f array
    // change the range from [0, 255] into [-1, 1]
    if (normals_texture.data() != NULL)
    {
        this->normals_map.init(normals_texture.width(), normals_texture.height());

        for (int i = 0; i < normals_texture.size(); i++)
        {
            Vector3f _n;
            _n = Vector3f
            (
                (float)normals_texture[i].r / 255.0,
                (float)normals_texture[i].g / 255.0,
                (float)normals_texture[i].b / 255.0
            );
            _n = Vector3f((_n.x * 2.0) - 1.0, (_n.y * 2.0) - 1.0, (_n.z * 2.0) - 1.0);
            this->normals_map[i] = normalize(_n);
        }

        normals_texture.release();
    }

    return true;
}

void Model::release()
{
    this->texture_map.release();
    this->specular_map.release();
    this->normals_map.release();
    this->mesh.release();
}

// normalize an array of 3D Vectors by the longest magnitude in the array
void Util::normalize(Array<Vector3f> &mesh)
{
    if (mesh.size() == 0) return;

    float max_length = 0;

    for (int i = 0; i < mesh.size(); i++)
    {
        max_length = _max(max_length, mesh[i].length());
    }
    for (int j = 0; j < mesh.size(); j++)
    {
        mesh[j].x = mesh[j].x / max_length;
        mesh[j].y = mesh[j].y / max_length;
        mesh[j].z = mesh[j].z / max_length;
    }
}

// smmoth an image by taking the average of each 3x3 block
void Util::smoothImage(FrameBuffer &buffer)
{
    if(buffer.data() == NULL) return;

    FrameBuffer temp(buffer.width(), buffer.height());

    for(int i = 1 ; i < buffer.width() - 1 ; i++) // calculating average colors for smoothing
    {
        for(int j = 1 ; j < buffer.height() - 1 ; j++)
        {
            Color center      = buffer(i, j);
            Color up          = buffer(i, j + 1);
            Color down        = buffer(i, j - 1);
            Color right       = buffer(i + 1, j);
            Color left        = buffer(i - 1, j);
            Color up_left     = buffer(i - 1, j + 1);
            Color up_right    = buffer(i + 1, j + 1);
            Color down_left   = buffer(i - 1, j - 1);
            Color down_right  = buffer(i + 1, j - 1);

            temp(i, j).r = center.r/2 + (up.r+down.r+right.r+left.r + up_left.r+up_right.r+down_left.r+down_right.r)/16;
            temp(i, j).g = center.g/2 + (up.g+down.g+right.g+left.g + up_left.g+up_right.g+down_left.g+down_right.g)/16;
            temp(i, j).b = center.b/2 + (up.b+down.b+right.b+left.b + up_left.b+up_right.b+down_left.b+down_right.b)/16;
        }
    }

    // copy the final pixels
    for(int k = 0 ; k < buffer.size() ; k++)
    {
        buffer[k] = temp[k];
    }

    temp.release();
}

