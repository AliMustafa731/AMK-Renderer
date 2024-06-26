
// enable window visual theme style
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "program/program.h"
#include "geometry/geometry.h"
#include "graphics/rasterizator.h"
#include "data/loaders.h"
#include "common.h"

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

const char* header_txt = "\nBy : Ali Mustafa Kamel\n2022-2023\n\nUse Arrow keys for rotating\nUse W and S for zooming";

//---------------------------------
//   Constructors
//---------------------------------

Program::Program()
{

}

Program::Program(const char* name, int _w, int _h)
{
    init(name, _w, _h);
}

void Program::init(const char* name, int _w, int _h)
{
    // set control flags
    this->main_program = this;
    this->is_running = true;
    this->is_loading = false;
    this->need_update = true;

    // create and register the window

    WNDCLASS wc = {0};
    this->program_title = (char*)name;

    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = GetModuleHandle(NULL);
    wc.lpszClassName = "AMKRenderer";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);

    this->win_handle = CreateWindowEx
    (
        0, "AMKRenderer", name, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, _w, _h, NULL, NULL, GetModuleHandle(NULL), NULL
    );

    if(this->win_handle == NULL)
    {
        MessageBox(NULL, "Error : can't initialize the program : \"win_handle is NULL\"", "Opss!", MB_OK);
        exit(0);
    }

    ShowWindow(this->win_handle, SW_SHOW);
    this->win_hdc = GetDC(this->win_handle);

    // initialize graphics

    RECT rect;
    GetClientRect(this->win_handle, &rect);

    this->width = rect.right - 200;
    this->height = rect.bottom;
    this->frameBuffer.init(width, height);
    this->background.init(width, height);
    this->zbuffer.init(width, height);

    this->bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
    this->bitmap_info.bmiHeader.biWidth = width;
    this->bitmap_info.bmiHeader.biHeight = height;
    this->bitmap_info.bmiHeader.biPlanes = 1;
    this->bitmap_info.bmiHeader.biBitCount = 32;
    this->bitmap_info.bmiHeader.biCompression = BI_RGB;

    this->timerInit();

    // setting up the background

    for(int i = 0 ; i < frameBuffer.size() ; i++)
    {
        Color c = Color(160, 160, 160);
        this->background[i] = c;
        this->frameBuffer[i] = c;

        // reset the depth map (z-buufer) to very small value
        this->zbuffer[i] = -FLT_MAX;
    }

    // load a test 3D model

    this->camera.lookAt(Vector3f(0, 0, 5), Vector3f(0, 0, 0), Vector3f(0, 1, 0));
    this->object.position = Vector3f(0.0, 0.0, 0.0);
    this->object.model = &this->main_model;

    this->loadMainModel("brick.obj");

    this->render_state.e_flat_shading = false;
    this->render_state.e_normals = true;
    this->render_state.e_render_mode = true;
    this->render_state.e_specular = true;
    this->render_state.e_texture = true;
    this->render_state.e_wireframe = false;
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
        frameRateLimit(60.0f);
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
        camera.camera_offset.z -= 0.2;
        camera.camera_offset.z = _max(camera.camera_offset.z, -3.0);
        need_update = true;
    }
    if (GetKeyState('S') & 0x8000)
    {
        camera.camera_offset.z += 0.2;
        camera.camera_offset.z = _min(camera.camera_offset.z, 2.0);
        need_update = true;
    }

    // render
    if (need_update)
    {
        need_update = false;

        if (angle_y > 6.283) angle_y = 0;
        if (angle_x > 6.283) angle_x = 0;

        camera.camera_matrix = mul_matrix(rotation_y(angle_y), rotation_x(angle_x));
        clearFrameBuffer();

        renderer.draw(object, camera, frameBuffer, zbuffer, render_state);
        StretchDIBits
        (
            win_hdc, 0, 0, frameBuffer.width(), frameBuffer.height(), 0, 0, frameBuffer.width(), frameBuffer.height(),
            (void*)frameBuffer.data(), &bitmap_info, DIB_RGB_COLORS, SRCCOPY
        );
    }

    // limiting the FPS
    frameRateLimit(60.0f);
}

void Program::clearFrameBuffer()
{
    for (int i = 0; i < frameBuffer.size(); i++)
    {
        frameBuffer[i] = background[i];

        // reset the depth map (z-buufer) to very small value
        zbuffer[i] = -FLT_MAX;
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
    CheckDlgButton(hwnd, ID_TEXTURE_MAPPING, render_state.e_texture);
    CheckDlgButton(hwnd, ID_NORMAL_MAPPING, render_state.e_normals);
    CheckDlgButton(hwnd, ID_SPECULAR_MAPPING, render_state.e_specular);
    CheckDlgButton(hwnd, ID_FLAT_SHADING, render_state.e_flat_shading);
    CheckDlgButton(hwnd, ID_WIREFRAME, render_state.e_wireframe);

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
}

void Program::onCommand(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int wmId = LOWORD(wParam);

    if (wmId == ID_TEXTURE_MAPPING) render_state.e_texture = IsDlgButtonChecked(hwnd, wmId);
    if (wmId == ID_NORMAL_MAPPING) render_state.e_normals = IsDlgButtonChecked(hwnd, wmId);
    if (wmId == ID_SPECULAR_MAPPING) render_state.e_specular = IsDlgButtonChecked(hwnd, wmId);
    if (wmId == ID_WIREFRAME) render_state.e_wireframe = IsDlgButtonChecked(hwnd, wmId);
    if (wmId == ID_FLAT_SHADING) render_state.e_flat_shading = IsDlgButtonChecked(hwnd, wmId);

    render_state.e_render_mode = IsDlgButtonChecked(hwnd, ID_RENDER_MODE);

    if (wmId == ID_LOAD_FILE)
    {
        if (IsWindowEnabled(GetDlgItem(hwnd, wmId)))
        {
            if (OpenFileDialog(".obj 3D format\0 * .obj\0\0"))
            {
                this->is_loading = true;
                EnableAllButtons(false);

                _beginthread(load_model_thread, 0, (void*)explorer_file_path);
            }
        }
    }
    if (wmId == ID_SAVE_IMAGE)
    {
        if (IsWindowEnabled(GetDlgItem(hwnd, wmId)))
        {
            if (SaveFileDialog(".png format\0*.png\0\0", "png"))
            {
                if (SaveImageData(explorer_file_path, this->frameBuffer))
                {
                    std::string str = "Successfully saved image to :\n" + std::string(explorer_file_path);
                    MessageBox(hwnd, str.c_str(), "Done", MB_OK);
                }
            }
        }
    }

    Program::main_program->need_update = true;
    SetFocus(hwnd);
}

void Program::onKeyDown(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (render_state.e_render_mode && !(Program::main_program->is_loading))
    {
        if (wParam == VK_SPACE && IsWindowEnabled(GetDlgItem(hwnd, ID_WIREFRAME)))
        {
            render_state.e_wireframe = !(render_state.e_wireframe);
            CheckDlgButton(hwnd, ID_WIREFRAME, render_state.e_wireframe);
        }
        else if (wParam == 'N' && IsWindowEnabled(GetDlgItem(hwnd, ID_NORMAL_MAPPING)))
        {
            render_state.e_normals = !(render_state.e_normals);
            CheckDlgButton(hwnd, ID_NORMAL_MAPPING, render_state.e_normals);
        }
        else if (wParam == 'P' && IsWindowEnabled(GetDlgItem(hwnd, ID_SPECULAR_MAPPING)))
        {
            render_state.e_specular = !(render_state.e_specular);
            CheckDlgButton(hwnd, ID_SPECULAR_MAPPING, render_state.e_specular);
        }
        else if (wParam == 'T' && IsWindowEnabled(GetDlgItem(hwnd, ID_TEXTURE_MAPPING)))
        {
            render_state.e_texture = !(render_state.e_texture);
            CheckDlgButton(hwnd, ID_TEXTURE_MAPPING, render_state.e_texture);
        }
        else if (wParam == 'F' && IsWindowEnabled(GetDlgItem(hwnd, ID_FLAT_SHADING)))
        {
            render_state.e_flat_shading = !(render_state.e_flat_shading);
            CheckDlgButton(hwnd, ID_FLAT_SHADING, render_state.e_flat_shading);
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
        (void*)Program::main_program->frameBuffer.data(), &Program::main_program->bitmap_info, DIB_RGB_COLORS, SRCCOPY
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
    Program::main_program->loadMainModel((const char*)args);
}

void Program::loadMainModel(const char* filename)
{
    this->main_model.release();

    this->main_model.loadFromOBJFile(filename);
    this->EnableAllButtons(true);

    // Enabling & Disabling controls depending on which textures the model has
    EnableWindow(GetDlgItem(win_handle, ID_TEXTURE_MAPPING), (main_model.texture_map.data() != NULL));
    EnableWindow(GetDlgItem(win_handle, ID_NORMAL_MAPPING), (main_model.normals_map.data() != NULL));
    EnableWindow(GetDlgItem(win_handle, ID_SPECULAR_MAPPING), (main_model.specular_map.data() != NULL));
    EnableWindow(GetDlgItem(win_handle, ID_FLAT_SHADING), (!main_model.flat_shading));

    this->is_loading = false;
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

int Program::OpenFileDialog(const char* filter, const char* title)
{
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(OPENFILENAME));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = this->win_handle;
    ofn.lpstrFile = (LPSTR)explorer_file_path;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = title;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

    return GetOpenFileName(&ofn);
}

int Program::SaveFileDialog(const char* filter, const char* default_ext, const char* title)
{
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(OPENFILENAME));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = this->win_handle;
    ofn.lpstrFile = (LPSTR)explorer_file_path;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrDefExt = default_ext;
    ofn.lpstrTitle = title;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    return GetSaveFileName(&ofn);
}

//--------------------------------------
//   Timer & frame-rate limiting
//--------------------------------------

void Program::timerInit()
{
    LARGE_INTEGER timer_freq;
    QueryPerformanceFrequency(&timer_freq);
    timer_frequency = (float)timer_freq.QuadPart / 1000.0f;
}

void Program::frameRateLimit(float fps)
{
    int frame_time = (1.0f / fps) * 1000.0f; // time needed for one frame (in milliseconds)

    QueryPerformanceCounter(&last);

    float delta = (float)(last.QuadPart - first.QuadPart) / timer_frequency;
    float result = frame_time - delta;

    std::string txt = "AMK Renderer " + std::to_string((int)delta) + " ms";
    SetWindowText(Program::main_program->WindowHandle(), txt.c_str());

    if (result > 0)
    {
        Sleep(result);
    }

    QueryPerformanceCounter(&first);
}
