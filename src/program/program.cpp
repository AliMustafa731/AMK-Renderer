
// enable window visual theme style
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "program/program.h"
#include "geometry/geometry.h"
#include "graphics/rasterizator.h"
#include "data/loaders.h"
#include "common.h"
#include "program/timer.h"

#include <process.h>
#include <cmath>
#include <string>


//---------------------------------
//   Forward Declarations
//---------------------------------
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void load_model_thread(void *args);

//---------------------------------
//   Variables
//---------------------------------

Program* Program::main_program;
Model* Program::main_model;

Object object;
float angle_y = 0.0f; // angle around Y-axis
float angle_x = 0.4f; // angle around X-axis
const char* program_title;

HWND buttons[16];
OPENFILENAME open_file = { 0 };
OPENFILENAME save_file = { 0 };
char file_name[512];

const char* header_txt = "\nBy : Ali Mustafa Kamel\n2022-2023\n\nUse Arrow keys for rotating\nUse W and S for zooming";
const char* info_txt =
"This program was made with the goal of not using any 3rd-party graphics libraries. \
The goal was to learn how 3D graphics works at the most \"low-level\", which is drawing a single pixel to the screen.\n\n\
Note : win32 API was used here only to manage the window and it's controls\n\nSee \"README.txt\" for more information";

//---------------------------------
//   Constructors
//---------------------------------

Program::Program(){}

Program::Program(const char* name, int _w, int _h)
{
    init(name, _w, _h);
}

void Program::init(const char* name, int _w, int _h)
{
    // set control flags

    main_program = this;
    is_running = true;
    is_loading = false;
    need_update = true;

    // create and register the window

    WNDCLASS wc = {0};
    program_title = name;

    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = GetModuleHandle(NULL);
    wc.lpszClassName = "AMKRenderer";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);

    win_handle = CreateWindowEx
    (
        0, "AMKRenderer", name, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, _w, _h, NULL, NULL, GetModuleHandle(NULL), NULL
    );

    if(win_handle == NULL)
    {
        MessageBox(NULL, "Error : can't initialize the program : \"win_handle is NULL\"", "Opss!", MB_OK);
        exit(0);
    }

    ShowWindow(win_handle, SW_SHOW);
    win_hdc = GetDC(win_handle);

    // initialize graphics

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

    timer_init();

    // setting up the background

    for(int i = 0 ; i < screen.size() ; i++)
    {
        Color c(160, 160, 160);
        background[i] = c;
        screen[i] = c;
        zbuffer[i] = -FLT_MAX;  // set to a small value
    }

    // load a test 3D model

    lookAt(Vector3(0, 0, 5), Vector3(0, 0, 0), Vector3(0, 1, 0));
    main_model = new Model();
    object.position = Vector3(0.0, 0.0, 0.0);
    object.model = main_model;

    loadFromOBJFile("brick.obj", main_model);
    EnableButtonsOnModel(main_model);
}

//---------------------------------
//   Program's Logic
//---------------------------------

void Program::update()
{
    // handle system input
    MSG msg = {};
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
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
    if (GetKeyState('W') & 0x8000)
    {
        camera_offset.z -= 0.2;
        camera_offset.z = _max(camera_offset.z, -3.0);
        need_update = true;
    }
    if (GetKeyState('S') & 0x8000)
    {
        camera_offset.z += 0.2;
        camera_offset.z = _min(camera_offset.z, 2.0);
        need_update = true;
    }

    // render
    if (need_update)
    {
        need_update = false;

        if (angle_y > 6.283) angle_y = 0;
        if (angle_x > 6.283) angle_x = 0;

        camera_matrix = mul_matrix(rotation_y(angle_y), rotation_x(angle_x));
        clearScreen();

        draw(object, screen, zbuffer);
        StretchDIBits
        (
            win_hdc, 0, 0, screen.width, screen.height, 0, 0, screen.width, screen.height,
            (void*)screen.data, &bitmap_info, DIB_RGB_COLORS, SRCCOPY
        );
    }

    // limiting the FPS
    frameRateLimit();
}

void Program::clearScreen()
{
    for (int i = 0; i < screen.size(); i++)
    {
        screen[i] = background[i];
        zbuffer[i] = -FLT_MAX;  // set to a small value
    }
}

//---------------------------------
//   Main Window Callback
//---------------------------------

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
    {
        Program::main_program->onCreate(hwnd, uMsg, wParam, lParam);

    } return 0;

    case WM_COMMAND:
    {
        Program::main_program->onCommand(hwnd, uMsg, wParam, lParam);

    } return 0;

    case WM_KEYDOWN:
    {
        Program::main_program->onKeyDown(hwnd, uMsg, wParam, lParam);

    } return 0;

    case WM_PAINT:
    {
        Program::main_program->onDraw(hwnd, uMsg, wParam, lParam);

    } return 0;

    case WM_DESTROY:
    {
        Program::main_program->onDestroy(hwnd, uMsg, wParam, lParam);

    } return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

//---------------------------------
//  Windows Callback Functions
//---------------------------------

void Program::onCreate(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    for (int i = 0; i < 16; i++)
    {
        buttons[i] = NULL;
    }

    // creating buttons and controls
    CreateWindow
    (
        "STATIC", header_txt, WS_VISIBLE | WS_CHILD | SS_CENTER,
        800, 0, 180, 100, hwnd, NULL, GetModuleHandle(NULL), NULL
    );
    buttons[0] = CreateWindow
    (
        "BUTTON", "Load from file", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        830, 330, 120, 30, hwnd, (HMENU)ID_LOAD_FILE, GetModuleHandle(NULL), NULL
    );
    buttons[1] = CreateWindow
    (
        "BUTTON", "Save as image", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        830, 380, 120, 30, hwnd, (HMENU)ID_SAVE_IMAGE, GetModuleHandle(NULL), NULL
    );
    buttons[2] = CreateWindow
    (
        "BUTTON", "More info", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        830, 530, 120, 30, hwnd, (HMENU)ID_INFO, GetModuleHandle(NULL), NULL
    );
    buttons[3] = CreateWindow
    (
        "BUTTON", "Texture mapping   \'T\'", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
        820, 170, 170, 30, hwnd, (HMENU)ID_TEXTURE_MAPPING, GetModuleHandle(NULL), NULL
    );
    buttons[4] = CreateWindow
    (
        "BUTTON", "Normals mapping  \'N\'", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
        820, 200, 170, 30, hwnd, (HMENU)ID_NORMAL_MAPPING, GetModuleHandle(NULL), NULL
    );
    buttons[5] = CreateWindow
    (
        "BUTTON", "Specular highlight  \'P\'", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
        820, 230, 170, 30, hwnd, (HMENU)ID_SPECULAR_MAPPING, GetModuleHandle(NULL), NULL
    );
    buttons[6] = CreateWindow
    (
        "BUTTON", "Flat shading  \'F\'", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
        820, 260, 170, 30, hwnd, (HMENU)ID_FLAT_SHADING, GetModuleHandle(NULL), NULL
    );
    buttons[7] = CreateWindow
    (
        "BUTTON", "Wireframe", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
        820, 290, 170, 30, hwnd, (HMENU)ID_WIREFRAME, GetModuleHandle(NULL), NULL
    );
    CheckDlgButton(hwnd, ID_TEXTURE_MAPPING, e_texture);
    CheckDlgButton(hwnd, ID_NORMAL_MAPPING, e_normals);
    CheckDlgButton(hwnd, ID_SPECULAR_MAPPING, e_specular);
    CheckDlgButton(hwnd, ID_FLAT_SHADING, e_flat_shading);
    CheckDlgButton(hwnd, ID_WIREFRAME, e_wireframe);

    buttons[8] = CreateWindow
    (
        "BUTTON", "Render", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
        810, 120, 70, 30, hwnd, (HMENU)ID_RENDER_MODE, GetModuleHandle(NULL), NULL
    );
    buttons[9] = CreateWindow
    (
        "BUTTON", "Wireframe", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
        880, 120, 100, 30, hwnd, (HMENU)ID_WIREFRAME_MODE, GetModuleHandle(NULL), NULL
    );
    CheckRadioButton(hwnd, ID_RENDER_MODE, ID_WIREFRAME_MODE, ID_RENDER_MODE);

    // Initialize OPENFILENAME for loading and saving
    ZeroMemory(&open_file, sizeof(OPENFILENAME));
    ZeroMemory(&save_file, sizeof(OPENFILENAME));

    open_file.lStructSize = sizeof(open_file);
    open_file.hwndOwner = hwnd;
    open_file.lpstrFile = file_name;
    open_file.lpstrFile[0] = '\0';
    open_file.nMaxFile = sizeof(file_name);
    open_file.lpstrFilter = ".obj 3D format\0*.obj\0\0";
    open_file.nFilterIndex = 1;
    open_file.lpstrFileTitle = NULL;
    open_file.nMaxFileTitle = 0;
    open_file.lpstrInitialDir = NULL;
    open_file.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

    save_file.lStructSize = sizeof(open_file);
    save_file.hwndOwner = hwnd;
    save_file.lpstrFile = file_name;
    save_file.lpstrFile[0] = '\0';
    save_file.nMaxFile = sizeof(file_name);
    save_file.lpstrFilter = ".png format\0*.png\0\0";
    save_file.nFilterIndex = 1;
    open_file.lpstrFileTitle = NULL;
    save_file.nMaxFileTitle = 0;
    save_file.lpstrInitialDir = NULL;
    save_file.lpstrDefExt = "png";
    save_file.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
}

void Program::onCommand(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int wmId = LOWORD(wParam);

    if (wmId == ID_TEXTURE_MAPPING) e_texture = IsDlgButtonChecked(hwnd, wmId);
    if (wmId == ID_NORMAL_MAPPING) e_normals = IsDlgButtonChecked(hwnd, wmId);
    if (wmId == ID_SPECULAR_MAPPING) e_specular = IsDlgButtonChecked(hwnd, wmId);
    if (wmId == ID_WIREFRAME) e_wireframe = IsDlgButtonChecked(hwnd, wmId);
    if (wmId == ID_FLAT_SHADING) e_flat_shading = IsDlgButtonChecked(hwnd, wmId);
    if (wmId == ID_INFO) MessageBox(hwnd, info_txt, "More info", MB_OK);

    e_render_mode = IsDlgButtonChecked(hwnd, ID_RENDER_MODE);

    if (wmId == ID_LOAD_FILE)
    {
        if (IsWindowEnabled(GetDlgItem(hwnd, wmId)))
        {
            if (GetOpenFileName(&open_file))
            {
                Program::main_program->is_loading = true;
                EnableAllButtons(false);

                _beginthread(load_model_thread, 0, NULL);
            }
        }
    }
    if (wmId == ID_SAVE_IMAGE)
    {
        if (IsWindowEnabled(GetDlgItem(hwnd, wmId)))
        {
            if (GetSaveFileName(&save_file))
            {
                if (SaveImageData(file_name, Program::main_program->screen))
                {
                    std::string _str_ = "Successfully saved image to :\n";
                    _str_ += file_name;
                    MessageBox(hwnd, _str_.c_str(), "Done", MB_OK);
                }
            }
        }
    }

    Program::main_program->need_update = true;
    SetFocus(hwnd);
}

void Program::onKeyDown(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (e_render_mode && !(Program::main_program->is_loading))
    {
        if (wParam == VK_SPACE && IsWindowEnabled(GetDlgItem(hwnd, ID_WIREFRAME)))
        {
            e_wireframe = !(e_wireframe);
            CheckDlgButton(hwnd, ID_WIREFRAME, e_wireframe);
        }
        else if (wParam == 'N' && IsWindowEnabled(GetDlgItem(hwnd, ID_NORMAL_MAPPING)))
        {
            e_normals = !(e_normals);
            CheckDlgButton(hwnd, ID_NORMAL_MAPPING, e_normals);
        }
        else if (wParam == 'P' && IsWindowEnabled(GetDlgItem(hwnd, ID_SPECULAR_MAPPING)))
        {
            e_specular = !(e_specular);
            CheckDlgButton(hwnd, ID_SPECULAR_MAPPING, e_specular);
        }
        else if (wParam == 'T' && IsWindowEnabled(GetDlgItem(hwnd, ID_TEXTURE_MAPPING)))
        {
            e_texture = !(e_texture);
            CheckDlgButton(hwnd, ID_TEXTURE_MAPPING, e_texture);
        }
        else if (wParam == 'F' && IsWindowEnabled(GetDlgItem(hwnd, ID_FLAT_SHADING)))
        {
            e_flat_shading = !(e_flat_shading);
            CheckDlgButton(hwnd, ID_FLAT_SHADING, e_flat_shading);
        }
    }

    Program::main_program->need_update = true;
}

void Program::onDraw(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    FillRect(hdc, &ps.rcPaint, (HBRUSH)COLOR_WINDOW);
    StretchDIBits
    (
        hdc, 0, 0, Program::main_program->width, Program::main_program->height,
        0, 0, Program::main_program->width, Program::main_program->height,
        (void*)Program::main_program->screen.data, &Program::main_program->bitmap_info, DIB_RGB_COLORS, SRCCOPY
    );

    EndPaint(hwnd, &ps);
    ReleaseDC(hwnd, hdc);
}

void Program::onDestroy(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    Program::main_program->is_running = false;
    PostQuitMessage(0);
}

//---------------------------------
//   Utilities
//---------------------------------

void load_model_thread(void *args)
{
    Program::main_model->release();

    loadFromOBJFile(file_name, Program::main_model);

    Program::main_program->EnableAllButtons(true);
    Program::main_program->EnableButtonsOnModel(Program::main_model);

    Program::main_program->is_loading = false;
}

void Program::EnableButtonsOnModel(Model *m)
{
    // Enabling & Disabling controls depending on which textures the model has
    EnableWindow(GetDlgItem(win_handle, ID_TEXTURE_MAPPING), (m->texture_map.data != NULL));
    EnableWindow(GetDlgItem(win_handle, ID_NORMAL_MAPPING), (m->normals_map.data != NULL));
    EnableWindow(GetDlgItem(win_handle, ID_SPECULAR_MAPPING), (m->specular_map.data != NULL));
    EnableWindow(GetDlgItem(win_handle, ID_FLAT_SHADING), (!m->flat_shading));
}

void Program::EnableAllButtons(bool state)
{
    for (int i = 0; i < 16; i++)
    {
        if (buttons[i] != NULL)
        {
            EnableWindow(buttons[i], state);
        }
    }
}
