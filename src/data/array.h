#ifndef array_included
#define array_included

#include <cassert>
#include <cstring>
#include <cstdint>
#include "common.h"

//---------------------------------
//   Dynamic Array Structure
//---------------------------------
template<typename T> class Array
{
public:

    Array() : _capacity(0), _size(0), _data(NULL) {}

    Array(size_t __capacity, T* __data = NULL) { init(__capacity, __data); }

    inline size_t capacity() const { return _capacity; }
    inline size_t size() const { return _size; }
    inline T* data() const { return _data; }

    inline T operator[](size_t i) const { AMK_ASSERT(i < _capacity);  return _data[i]; }
    inline T &operator[](size_t i) { AMK_ASSERT(i < _capacity);  return _data[i]; }

    void init(size_t __capacity, T* __data = NULL)
    {
        this->_capacity = __capacity;
        this->_size = _capacity;

        if (__data != NULL)
        {
            this->_data = __data;
        }
        else
        {
            _data = new T[_capacity];
            std::memset(_data, 0, _capacity * sizeof(T));
        }
    }

    void release()
    {
        if (_data != NULL)
        {
            delete[] _data;
            _data = NULL;
            _capacity = 0;
            _size = 0;
        }
    }

    void resize(size_t __size)
    {
        release();
        init(__size);
    }

    void reserve(size_t __size)
    {
        release();
        init(__size);
        _size = 0;
    }

    void add(T val)
    {
        if (_data == NULL)
        {
            reserve(5);
        }

        // if length eceeds the capacity
            // realloctate double the amount of memory
        if (_size >= _capacity)
        {
            // initialize & copy new buffer
            _capacity *= 2;
            T* tmp = new T[_capacity];

            for (int i = 0; i < _size; i++) { tmp[i] = _data[i]; }

            delete[] _data;
            _data = tmp;
        }

        _data[_size] = val;
        _size++;
    }

protected:
    T* _data;
    size_t _capacity, _size;
};

//----------------------------------------------
//   2-Dimensional Dynamic Array Structure
//----------------------------------------------
template<typename T> class Buffer : public Array<T>
{
public:

    Buffer()
    {
        this->_data = NULL;
        this->_size = 0;
        this->_capacity = 0;
        this->_width = 0;
        this->_height = 0;
    }

    Buffer(size_t w, size_t h) { init(w, h); }

    inline T operator()(size_t x, size_t y) const { AMK_ASSERT(x < _width && y < _height);  return this->_data[x + y * _width]; }
    inline T &operator()(size_t x, size_t y) { AMK_ASSERT(x < _width && y < _height);  return this->_data[x + y * _width]; }

    inline size_t width() const { return _width; }
    inline size_t height() const { return _height; }

    void init(size_t w, size_t h)
    {
        _width = w;
        _height = h;
        
        Array<T>::init(w * h);
    }

    void release()
    {
        Array<T>::release();
        _width = 0;
        _height = 0;
    }

private:
    size_t _width, _height;
};

#endif  // array_included