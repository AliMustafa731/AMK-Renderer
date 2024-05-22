#ifndef program_h
#define program_h

#include <Windows.h>
#include "graphics/graphics.h"

// ID's for buttons and controls

enum ControlID
{
    ID_TEXTURE_MAPPING,
    ID_NORMAL_MAPPING,
    ID_WIREFRAME,
    ID_FLAT_SHADING,
    ID_SPECULAR_MAPPING,
    ID_LOAD_FILE,
    ID_SAVE_IMAGE,
    ID_INFO,
    ID_RENDER_MODE,
    ID_WIREFRAME_MODE
};

class Program
{
public:

    Program();
    Program(const char* name, int _w, int _h);

    //---------------------------------
    //   Program's Logic
    //---------------------------------
    void init(const char* name, int _w, int _h);
    void update();
    void clearScreen();

    //---------------------------------
    //   Callbacks
    //---------------------------------
    void onCreate(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void onCommand(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void onKeyDown(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void onDraw(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void onDestroy(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void EnableButtonsOnModel(Model *m);
    void EnableAllButtons(bool state);

    //---------------------------------
    //   Variables
    //---------------------------------

    // global pointers
    static Program* main_program;
    static Model* main_model;

    // control flags
    bool is_running, is_loading, need_update;

    // graphics
    int width, height;
    ZBuffer zbuffer;
    FrameBuffer screen, background;
    // windows specific handles
    HDC win_hdc;
    HWND win_handle;
    BITMAPINFO bitmap_info;
};

#endif // program_h
