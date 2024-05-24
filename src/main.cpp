#include "program/program.h"


int main()
{
    Program program("AMK Renderer ", 1000, 640);

    while(program.is_running)
    {
        program.update();
    }

    return 0;
}

