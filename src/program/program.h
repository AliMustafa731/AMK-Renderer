#ifndef program_h
#define program_h

#include <Windows.h>
#include "graphics/graphics.h"
#include "graphics/rasterizator.h"

// ID's for buttons and controls

enum ControlID
{
    ID_TEXTURE_MAPPING = 1001,
    ID_NORMAL_MAPPING,
    ID_WIREFRAME,
    ID_FLAT_SHADING,
    ID_SPECULAR_MAPPING,
    ID_LOAD_FILE,
    ID_SAVE_IMAGE,
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
    void clearFrameBuffer();

    //---------------------------------
    //   Callbacks
    //---------------------------------
    void onCreate(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void onCommand(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void onKeyDown(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void onDraw(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void onDestroy(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void loadMainModel(const char* filename);  // called by an external thread

    //---------------------------------
    //   Utilities
    //---------------------------------
    int OpenFileDialog(const char* filter, const char* title = "Open File");
    int SaveFileDialog(const char* filter, const char* default_ext, const char* title = "Open File");
    void EnableAllButtons(bool state);

    //---------------------------------
    //   Accessors
    //---------------------------------
    HWND WindowHandle() const { return win_handle; }
    bool isRunning() const { return is_running; }

    // global pointer
    static Program* main_program;

private:

    //-----------------------------------------
    // timer (used to limit the frame-rate)
    //-----------------------------------------
    void frameRateLimit(float fps);
    void timerInit();

    float timer_frequency;
    LARGE_INTEGER first = { 0 };
    LARGE_INTEGER last = { 0 };

    // control flags
    bool is_running, is_loading, need_update;

    // camera & renderer
    Camera camera;
    Rasterizor renderer;
    RenderState render_state;

    // graphics
    int width, height;
    ZBuffer zbuffer;
    FrameBuffer frameBuffer, background;

    Model main_model;
    Object object;
    float angle_y = 0.0f; // angle around Y-axis
    float angle_x = 0.4f; // angle around X-axis

    // windows specific handles
    HDC win_hdc;
    HWND win_handle;
    BITMAPINFO bitmap_info;
    HWND buttons[16];

    // used to hold the path returned from OpenFileDialog & SaveFileDialog
    char explorer_file_path[MAX_PATH];
    char* program_title;
};

#endif // program_h
