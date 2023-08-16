#include "program.h"
#include "system.h"
#include "geometry.h"
#include "rasterizator.h"
#include "keycodes.h"
#include "common.h"
#include "loaders.h"
#include <cmath>
#include <string>


Program *main_program;
Model main_model;
Object object;
float angle_y = 0.0f; // angle around Y-axis
float angle_x = 0.4f; // angle around X-axis
const char* program_title;


void Program::init(const char* name, int _w, int _h)
{
    main_program = this;
    is_running = true;
    is_loading = false;
    need_update = true;
    WNDCLASS wc = {0};
    program_title = name;

    // create and register the window
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = GetModuleHandle(NULL);
    wc.lpszClassName = "AMKRenderer";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);

    win_handle = CreateWindowEx(0, "AMKRenderer", name, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, _w, _h, NULL, NULL, GetModuleHandle(NULL), NULL);

    if(win_handle == NULL)
    {
        MessageBox(NULL, "Error : can't initialize the program : \"phwnd\"", "Opss!", MB_OK);
        exit(0);
    }

    ShowWindow(win_handle, SW_SHOW);
    win_hdc = GetDC(win_handle);
    RECT rect;
    GetClientRect(win_handle, &rect);

    width = rect.right - 200;
    height = rect.bottom;
    screen.init(width, height);
    background.init(width, height);
    zbuffer.init(width, height);

    bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
    bitmap_info.bmiHeader.biWidth = width;
    bitmap_info.bmiHeader.biHeight = height;
    bitmap_info.bmiHeader.biPlanes = 1;
    bitmap_info.bmiHeader.biBitCount = 32;
    bitmap_info.bmiHeader.biCompression = BI_RGB;

    // setting up the background
    for(int i = 0 ; i < screen.size ; i++)
    {
        int y = i / width;
        float x = (float)y / height;
        unsigned char a = 40 + 160 * x;
        Color c(a, a, a);
        background[i] = c;
        screen[i] = c;
        zbuffer[i] = -FLT_MAX;  // set to a small value
    }

    // test
    lookAt(Vector3(0, 0, 5), Vector3(0, 0, 0), Vector3(0, 1, 0));
    object.position = Vector3(0.0, 0.0, 0.0);
    object.model = &main_model;

    loadFromOBJFile("brick.obj", &main_model);
    EnableButtonsOnModel(&main_model);
}

void frameRateLimit(); // forward declaration

void Program::update()
{
    // handle system input
    MSG msg = {};
    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (is_loading)
    {
        frameRateLimit();
        return;
    }

    if (GetKeyState(VK_LEFT) & 0x8000)
    {
        angle_y -= 0.05;
        need_update = true;
    }
    if (GetKeyState(VK_RIGHT) & 0x8000)
    {
        angle_y += 0.05;
        need_update = true;
    }
    if (GetKeyState(VK_UP) & 0x8000)
    {
        angle_x -= 0.05;
        need_update = true;
    }
    if (GetKeyState(VK_DOWN) & 0x8000)
    {
        angle_x += 0.05;
        need_update = true;
    }
    if (GetKeyState(VK_KEY_W) & 0x8000)
    {
        camera_offset.z -= 0.2;
        camera_offset.z = _max(camera_offset.z, -3.0);
        need_update = true;
    }
    if (GetKeyState(VK_KEY_S) & 0x8000)
    {
        camera_offset.z += 0.2;
        camera_offset.z = _min(camera_offset.z, 2.0);
        need_update = true;
    }

    // render
    if(need_update)
    {
        need_update = false;

        if(angle_y > 6.283) angle_y = 0;
        if(angle_x > 6.283) angle_x = 0;

        camera_matrix = MultiplyMatrix(rotation_y(angle_y), rotation_x(angle_x));
        clearScreen();

        draw(object, screen, zbuffer);
        StretchDIBits
        (
            win_hdc, 0, 0, screen.width, screen.height, 0, 0,screen.width, screen.height,
            (void*)screen.data, &bitmap_info, DIB_RGB_COLORS, SRCCOPY
        );
    }

    // limiting the FPS
    frameRateLimit();
}

void Program::clearScreen()
{
    for(int i = 0 ; i < screen.size ; i++)
    {
        screen[i] = background[i];
        zbuffer[i] = -FLT_MAX;  // set to a small value
    }
}

int first_time = 0, last_time = 0, ms = 0, ms_avg = 0;
std::string txt;

void frameRateLimit() // limiting the framerate to ~ 60 fps (58 actually)
{
    last_time = GetTickCount64();
    ms = last_time - first_time;
    int result = 17 - ms;

    if(result > 0)
    {
        Sleep(result);
    }

    txt = program_title + std::to_string((ms + ms_avg)/2) + " ms";
    ms_avg = ms;
    SetWindowText(main_program->win_handle, txt.c_str());

    first_time = GetTickCount64();
}


