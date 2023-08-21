#ifndef loaders_h
#define loaders_h

#include "graphics.h"

bool loadFromOBJFile(const char* filename, Model *m);

bool loadImageData(const char* filename, FrameBuffer &buffer);

int SaveImageData(const char* filename, FrameBuffer &buffer);

#endif // loaders_h
