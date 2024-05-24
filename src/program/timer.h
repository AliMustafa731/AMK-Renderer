#ifndef timer_included
#define timer_included

#include <string>
#include "program/program.h"


int Target_FPS = (1.0f / 60.0f) * 1000.0f; // time needed for one frame (in milliseconds)
float timer_frequency;
LARGE_INTEGER first = { 0 };
LARGE_INTEGER last = { 0 };

void timer_init()
{
    LARGE_INTEGER timer_freq;
    QueryPerformanceFrequency(&timer_freq);
    timer_frequency = (float)timer_freq.QuadPart / 1000.0f;
}

void frameRateLimit() // limiting the framerate to ~ 60 fps
{
    QueryPerformanceCounter(&last);

    float delta = (float)(last.QuadPart - first.QuadPart) / timer_frequency;
    float result = Target_FPS - delta;

    std::string txt = "AMK Renderer " + std::to_string((int)delta) + " ms";
    SetWindowText(Program::main_program->WindowHandle(), txt.c_str());

    if (result > 0)
    {
        Sleep(result);
    }

    QueryPerformanceCounter(&first);
}

#endif // timer_included