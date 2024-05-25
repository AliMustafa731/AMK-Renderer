#ifndef loaders_h
#define loaders_h

#include "graphics/graphics.h"

void Triangulate(VertexIndex* points, int count, Array<FaceIndex> &dest);

bool loadImageData(const char* filename, FrameBuffer &buffer);

int SaveImageData(const char* filename, FrameBuffer &buffer);

#endif // loaders_h
