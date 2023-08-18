#include "system.h"
#include "program.h"
#include "graphics.h"
#include "loaders.h"
#include "rasterizator.h"
#include "keycodes.h"
#include <process.h>

// enable window visual theme style
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


void onCreate(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void onCommand(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void onKeyDown(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void onDraw(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// main window callback
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CREATE:
        {
            onCreate(hwnd, uMsg, wParam, lParam);

        } return 0;

        case WM_COMMAND:
        {
            onCommand(hwnd, uMsg, wParam, lParam);

        } return 0;

        case WM_KEYDOWN:
        {
            onKeyDown(hwnd, uMsg, wParam, lParam);

        } return 0;

        case WM_PAINT:
        {
            onDraw(hwnd, uMsg, wParam, lParam);

        } return 0;

        case WM_DESTROY:
        {
            main_program->is_running = false;
            PostQuitMessage(0);

        } return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


const char* header_txt = "\nBy : Ali Mustafa Kamel\n2022-2023\n\nUse Arrow keys for rotating\nUse W and S for zooming";
const char* info_txt =
"The actual code for rendering is located in files :\n- rasterizator.cpp\n- graphics.cpp\n\n\
This program was made with the goal not to use any 3rd-party graphics libraries. \
The goal is to learn how 3D graphics work at the most \"low-level\" which is drawing a single pixel to the screen\n\n\
Note : win32 API was used here only to manage the window and it's controls\n\nSee \"README.txt\" for more information";

HWND buttons[16];

OPENFILENAME open_file = { 0 };
OPENFILENAME save_file = { 0 };
char file_name[512];

void EnableButtonsOnModel(Model *m)
{
    // Enabling & Disabling controls depending on which textures the model has
    EnableWindow(GetDlgItem(main_program->win_handle, ID_TEXTURE_MAPPING), (m->texture.data != NULL));
    EnableWindow(GetDlgItem(main_program->win_handle, ID_NORMAL_MAPPING), (m->normals_map.data != NULL));
    EnableWindow(GetDlgItem(main_program->win_handle, ID_SPECULAR_MAPPING), (m->specular.data != NULL));
    EnableWindow(GetDlgItem(main_program->win_handle, ID_FLAT_SHADING), (!m->flat_shading));
}

void EnableAllButtons(bool state)
{
    for (int i = 0; i < 16; i++)
    {
        if (buttons[i] != NULL)
        {
            EnableWindow(buttons[i], state);
        }
    }
}

void load_model_thread(void *args)
{
    main_model.release();

    loadFromOBJFile(file_name, &main_model);

    EnableAllButtons(true);
    EnableButtonsOnModel(&main_model);
    main_program->is_loading = false;
}

void onCreate(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

void onCommand(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
                main_program->is_loading = true;
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
                if (SaveImageData(file_name, main_program->screen))
                {
                    std::string _str_ = "Successfully saved image to :\n";
                    _str_ += file_name;
                    MessageBox(hwnd, _str_.c_str(), "Done", MB_OK);
                }
            }
        }
    }

    main_program->need_update = true;
    SetFocus(hwnd);
}

void onKeyDown(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (e_render_mode && !(main_program->is_loading))
    {
        if (wParam == VK_SPACE && IsWindowEnabled(GetDlgItem(hwnd, ID_WIREFRAME)))
        {
            e_wireframe = !(e_wireframe);
            CheckDlgButton(hwnd, ID_WIREFRAME, e_wireframe);
        }
        else if (wParam == VK_KEY_N && IsWindowEnabled(GetDlgItem(hwnd, ID_NORMAL_MAPPING)))
        {
            e_normals = !(e_normals);
            CheckDlgButton(hwnd, ID_NORMAL_MAPPING, e_normals);
        }
        else if (wParam == VK_KEY_P && IsWindowEnabled(GetDlgItem(hwnd, ID_SPECULAR_MAPPING)))
        {
            e_specular = !(e_specular);
            CheckDlgButton(hwnd, ID_SPECULAR_MAPPING, e_specular);
        }
        else if (wParam == VK_KEY_T && IsWindowEnabled(GetDlgItem(hwnd, ID_TEXTURE_MAPPING)))
        {
            e_texture = !(e_texture);
            CheckDlgButton(hwnd, ID_TEXTURE_MAPPING, e_texture);
        }
        else if (wParam == VK_KEY_F && IsWindowEnabled(GetDlgItem(hwnd, ID_FLAT_SHADING)))
        {
            e_flat_shading = !(e_flat_shading);
            CheckDlgButton(hwnd, ID_FLAT_SHADING, e_flat_shading);
        }
    }

    main_program->need_update = true;
}

void onDraw(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    FillRect(hdc, &ps.rcPaint, (HBRUSH)COLOR_WINDOW);
    StretchDIBits
    (
        hdc, 0, 0, main_program->width, main_program->height, 0, 0, main_program->width, main_program->height,
        (void*)main_program->screen.data, &main_program->bitmap_info, DIB_RGB_COLORS, SRCCOPY
    );

    EndPaint(hwnd, &ps);
    ReleaseDC(hwnd, hdc);
}

