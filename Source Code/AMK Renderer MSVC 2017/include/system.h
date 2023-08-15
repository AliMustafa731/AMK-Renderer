#ifndef system_h
#define system_h

#include <Windows.h>

// ID's for buttons and controls
#define ID_TEXTURE_MAPPING 1
#define ID_NORMAL_MAPPING 2
#define ID_WIREFRAME 3
#define ID_FLAT_SHADING 4
#define ID_SPECULAR_MAPPING 5
#define ID_LOAD_FILE 6
#define ID_SAVE_IMAGE 7
#define ID_INFO 8
#define ID_RENDER_MODE 9
#define ID_WIREFRAME_MODE 10

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

struct Model;  // forward declaration

void EnableButtonsOnModel(Model *m);

#endif // system_h
