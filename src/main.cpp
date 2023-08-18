#include "program.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow)
{
    Program program("AMK Renderer ", 1000, 640);

    while(program.is_running)
    {
        program.update();
    }

    return 0;
}

