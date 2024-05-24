#include "program/program.h"


int main()
{
    Program program("AMK Renderer ", 1000, 640);

    while(program.isRunning())
    {
        program.update();
    }

    return 0;
}

