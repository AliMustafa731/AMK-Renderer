#ifndef array_included
#define array_included

#include <cassert>
#include "common.h"

#ifndef NULL
#define NULL 0
#endif

template<typename T> struct Array
{
    int size, counter;
    T *data;

    inline T operator[](int i) const { AMK_ASSERT(i < size);  return data[i]; }
    inline T &operator[](int i)      { AMK_ASSERT(i < size);  return data[i]; }

    void init(int _size)
    {
        size = _size;
        counter = 0;
        data = new T[size];
    }

    void add(T &val)
    {
        data[counter] = val;
        counter++;
    }

    Array()
    {
        size = 0;
        counter = 0;
        data = NULL;
    }

    Array(int _size) { init(_size); }

    void release()
    {
        if (data != NULL)
        {
            delete[] data;
            data = NULL;
            size = 0;
            counter = 0;
        }
    }
};

template<typename T> struct Buffer
{
    int width, height, size;
    T *data;

    inline T operator[](int i) const { AMK_ASSERT(i < size);  return data[i]; }
    inline T &operator[](int i)      { AMK_ASSERT(i < size);  return data[i]; }

    inline T operator()(int x, int y) const { AMK_ASSERT(x < width && y < height);  return data[x + y * width]; }
    inline T &operator()(int x, int y)      { AMK_ASSERT(x < width && y < height);  return data[x + y * width]; }

    void init(int w, int h)
    {
        width = w;
        height = h;
        size = w * h;
        data = new T[size];
    }

    void release()
    {
        if (data != NULL)
        {
            delete[] data;
            data = NULL;
            width = 0;
            height = 0;
            size = 0;
        }
    }

    Buffer()
    {
        width = 0;
        height = 0;
        size = 0;
        data = NULL;
    }

    Buffer(int w, int h) { init(w, h); }
};

#endif