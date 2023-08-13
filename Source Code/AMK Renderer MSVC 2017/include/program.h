#ifndef program_h
#define program_h

#include <Windows.h>
#include "graphics.h"


struct Program
{
    bool is_running, is_loading, need_update;
    int width, height;
    ZBuffer zbuffer;
    FrameBuffer screen, background;
    HDC win_hdc;
    HWND win_handle;
    BITMAPINFO bitmap_info;

    Program(){}
    Program(const char* name, int _w, int _h)
    {
        init(name, _w, _h);
    }

    void init(const char* name, int _w, int _h);

    void update();

    void clearScreen();
};

extern Program *main_program;
extern Model main_model;


#endif // program_h
