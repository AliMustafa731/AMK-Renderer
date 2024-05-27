#ifndef loaders_h
#define loaders_h

#include "graphics/graphics.h"

// an algorithm to convert polygonal faces into triangles
void Triangulate(VertexIndex* points, int count, Array<FaceIndex> &dest);

// load images stored in (.jpeg, .png) from a file into a "FrameBuffer"
bool loadImageData(const char* filename, FrameBuffer &buffer);

// save images stored in a "FrameBuffer" to a (.jpeg, .png) file on disk 
int SaveImageData(const char* filename, FrameBuffer &buffer);

#endif // loaders_h
