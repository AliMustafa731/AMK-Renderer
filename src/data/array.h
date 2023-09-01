#ifndef array_included
#define array_included

#include <cassert>
#include "common.h"

//---------------------------------
//   Dynamic Array Structure
//   you must use "reserve()"
//   if you want to add elements
//---------------------------------
template<typename T> struct Array
{
    int _size, _length;
    T* data;

    Array() { _size = 0;  _length = 0;  data = NULL; }
    Array(int __size) { init(__size); }
    Array(int __size, T* _data) { init(__size, _data); }

    inline int capacity() const { return _size; }
    inline int size() const { return _length; }

    inline T operator[](int i) const { AMK_ASSERT(i < _size);  return data[i]; }
    inline T &operator[](int i) { AMK_ASSERT(i < _size);  return data[i]; }

    void init(int __size)
    {
        _size = __size;
        _length = _size;
        data = new T[_size];

        // initialize the memory to zero
        unsigned char* p = (unsigned char*)data;
        for (int i = 0; i < _size * sizeof(T); i++) { p[i] = 0; }
    }

    void init(int __size, T* _data)
    {
        _size = __size;
        _length = _size;
        data = _data;
    }

    void release()
    {
        if (data != NULL)
        {
            delete[] data;
            data = NULL;
            _size = 0;
            _length = 0;
        }
    }

    void resize(int __size)
    {
        release();
        init(__size);
    }

    void reserve(int __size)
    {
        release();
        init(__size);
        _length = 0;
    }

    void add(T val)
    {
        if (data == NULL) { return; }

        // if length eceeds the capacity
        // realloctate double the amount of memory
        if (_length >= _size)
        {
            _size *= 2;

            // initialize & copy new buffer
            T* tmp = new T[_size];
            for (int i = 0; i < _length; i++) { tmp[i] = data[i]; }

            delete[] data;
            data = tmp;
        }

        data[_length] = val;
        _length += 1;
    }
};

//------------------------------------
//   Dynamic Array Structure
//   With 2D Meta-info & accessors
//------------------------------------
template<typename T> struct Buffer
{
    int width, height, _size;
    T *data;

    inline T operator[](int i) const { AMK_ASSERT(i < size);  return data[i]; }
    inline T &operator[](int i) { AMK_ASSERT(i < size);  return data[i]; }

    inline T operator()(int x, int y) const { AMK_ASSERT(x < width && y < height);  return data[x + y * width]; }
    inline T &operator()(int x, int y) { AMK_ASSERT(x < width && y < height);  return data[x + y * width]; }

    inline int size() const { return _size; }

    void init(int w, int h)
    {
        width = w;
        height = h;
        _size = w * h;
        data = new T[_size];
    }

    void release()
    {
        if (data != NULL)
        {
            delete[] data;
            data = NULL;
            width = 0;
            height = 0;
            _size = 0;
        }
    }

    Buffer()
    {
        width = 0;
        height = 0;
        _size = 0;
        data = NULL;
    }

    Buffer(int w, int h) { init(w, h); }
};

#endif  // array_included