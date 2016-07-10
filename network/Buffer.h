#pragma once
#include <assert.h>
#include <string.h>
#include <wchar.h>

namespace Network
{

typedef unsigned char int8;

// 缓冲区基类(http://www.cppblog.com/PeakGao/archive/2007/06/07/25709.html)
class Buffer
{
public:
    enum bufstate{ _good = 0x0, _eof = 0x1, _fail = 0x2, _bad = 0x4/**//*严重错误*/};

protected:
    void*        _buf;        /**//// 缓冲区首地址
    size_t       _bufSize;    /**//// 缓冲区大小
    size_t       _pos;        /**//// 当前操作位置
    bufstate     _state;      /**//// 缓冲区当前操作的状态

    // 构造（析构）
protected:
    Buffer() : _buf(0), _bufSize(0), _pos(0), _state(_good) { }

public:
    Buffer(void* buffer, size_t maxsize) : _buf(buffer), _bufSize(maxsize), _pos(0), _state(_good)
    {
        assert(buffer != 0 && maxsize > 0);
    }

    virtual ~Buffer() { }

    // 状态相关（用于检测操作的结果）
public:
    bool operator !() const
    {
        return (!good());
    }

    operator bool() const
    {
        return (good());
    }

    bufstate state() const
    {
        return _state;
    }

    void setstate(bufstate state_)
    {
        if (state_ != _good)
            _state = (bufstate)((int)_state | (int)state_);
    }

    void setstate(int state_)
    {
        setstate((bufstate)state_);
    }

    bool good() const
    {
        return ((int)state() == (int)_good || (int)state() == (int)(_good | _eof));
    }

    bool eof() const
    {
        return ((int)state() & (int)_eof);
    }

    bool fail() const
    {
        return (((int)state() & ((int)_fail | (int)_bad)) != 0);
    }

    bool bad() const
    {
        return (((int)state() & (int)_bad) != 0);
    }


    // 属性及操作
public:
    /**//// 缓冲区清除操作
    virtual void clear();

    /**//// 将当前位置向后移动指定的大小
    virtual void skipn(size_t bytes);

    /**//// 获取缓冲区地址
    virtual const char* data() const
    {
        return (char*)_buf;
    }

    /**//// 获取缓冲区当前位置的地址
    virtual const char* current() const
    {
        return (char*)_buf + _pos;
    }

    /**//// 获取缓冲区数据操作的当前(读/写)位置偏移
    virtual size_t pos() const
    {
        return _pos;
    }

    /**//// 获取缓冲区剩余未操作(读/写)大小
    virtual size_t remains() const
    {
        return capacity() - pos();
    }

    /**//// 获取缓冲区的容量（即缓冲区的大小）
    virtual size_t capacity() const
    {
        return _bufSize;
    }

    /**//// 获取缓冲区中数据的实际占用尺寸
    virtual size_t size() const = 0;

    //将缓冲区前面bytes大小的数据移除掉
    virtual void shrink(size_t bytes) = 0;
};




/**//// 输出缓冲（指各种数据导入到缓冲区中）
class OutBuffer : public Buffer
{
    // 构造（析构）
protected:
    OutBuffer() : Buffer() {}

public:
    OutBuffer(void* buffer, size_t maxsize) : Buffer(buffer, maxsize) {}

    virtual ~OutBuffer() {}


    // 方法
public:
    /**//// 获取缓冲区中数据的实际占用尺寸
    size_t size() const;

    //将缓冲区前面bytes大小的数据移除掉
    virtual void shrink(size_t bytes);

    /**//// 向缓冲区写入各种具有固定长度的数据类型，包括简单类型和复合类型（结构）
    template <class T> OutBuffer& operator << (T value)
    {
        if (_pos + sizeof(T) <= _bufSize)
        {
            *(T*)((char*)_buf + _pos) = value;
            _pos += sizeof(T);

            if (_pos == _bufSize)
                setstate(Buffer::_eof);
        }
        else
        {
            setstate(Buffer::_eof|Buffer::_fail);
        }
        return (*this);
    }
    
    /**//// 对常字符串的特化处理
    OutBuffer& operator << (const char* value)
    {
        return push((void*)value, strlen(value) + sizeof(char));
    }
    
    /**//// 对字符串的特化处理
    OutBuffer& operator << (char* value)
    {
        return push((void*)value, strlen(value) + sizeof(char));
    }

    /**//// 对常宽字符串的特化处理
    OutBuffer& operator << (const wchar_t* value)
    {
        return push((void*)value, (wcslen(value) + 1) * sizeof(wchar_t));
    }

    /**//// 对宽字符串的特化处理
    OutBuffer& operator << (wchar_t* value)
    {
        return push((void*)value, (wcslen(value) + 1) * sizeof(wchar_t));
    }


    /**//// 跳过某种数据类型（不进行赋值，仅仅改变当前位置）
    template <class T> OutBuffer& skip()
    {
        if (_pos + sizeof(T) <= _bufSize)
        {
            _pos += sizeof(T);

            if (_pos == _bufSize)
                setstate(Buffer::_eof);
        }
        else
        {
            setstate(Buffer::_eof|Buffer::_fail);
        }
        return (*this);
    }

    /**//// 在当前位置导入一段缓冲
    OutBuffer& push(void* buffer, size_t bytes)
    {
        if (buffer == 0 || bytes == 0)
        {
            setstate(Buffer::_bad|Buffer::_fail);
            return (*this);
        }

        if (_pos + bytes <= _bufSize)
        {
            memcpy((char*)_buf + _pos, buffer, bytes);
            _pos += bytes;

            if (_pos == _bufSize)
                setstate(Buffer::_eof);
        }
        else
        {
            setstate(Buffer::_eof|Buffer::_fail);
        }
        return (*this);
    }
};


/**//// 固定大小的输出缓冲
template <size_t _MaxBytes = 256>
class FixOutBuffer : public OutBuffer
{
protected:
    enum {_Buf_Size = _MaxBytes ? ((_MaxBytes + 7) & ~7) : 8}; // 8字节对齐
    char _data[_Buf_Size];

public:
    FixOutBuffer() : OutBuffer(_data, _Buf_Size){}
    virtual ~FixOutBuffer() {}
};




/**//// 输入缓冲（指从缓冲区中导出各种数据）
class InBuffer : public Buffer
{
    // 构造（析构）
protected:
    InBuffer() : Buffer() { }

public:
    InBuffer(void* buffer, size_t actualSize) : Buffer(buffer, actualSize)    { }

    virtual ~InBuffer() { }


    // 方法
public:
    /**//// 获取缓冲区中数据的实际占用尺寸
    size_t size() const
    {
        return _bufSize;
    }

    virtual void shrink(size_t bytes)
    {
        return;
    }

    /**//// 从缓冲区导出各种具有固定长度的数据类型，包括简单类型和复合类型（结构）
    template <class T> InBuffer& operator >> (T& value)
    {
        if (_pos + sizeof(T) <= _bufSize)
        {
            value = *(T*)((char*)_buf + _pos);
            _pos += sizeof(T);

            if (_pos == _bufSize)
                setstate(Buffer::_eof);
        }
        else
        {
            setstate(Buffer::_eof|Buffer::_fail);
        }
        return (*this);
    }

    //使用GCC编译时,该段代码可能要移植到buffer.cpp文件中分离编译
    /**//// 对常字符串的特化处理
    InBuffer& operator >> (const char*& value)
    {
        const char* str = (const char*)_buf + _pos;
        while ((size_t)(str - (const char*)_buf) < _bufSize && *str++);
        size_t bytes = (size_t)(str - (char*)_buf) - _pos;
        if (bytes > 0 && _pos + bytes <= _bufSize)
        {
            if (*((char*)_buf + _pos + bytes - 1) != 0) // 不是0结尾的字符串
            {
                setstate(Buffer::_eof|Buffer::_bad);
                return (*this);
            }
            value = (char*)_buf + _pos;
            _pos += bytes;

            if (_pos == _bufSize)
                setstate(Buffer::_eof);
        }
        else
        {
            setstate(Buffer::_eof|Buffer::_fail);
            value = 0;
        }

        return (*this);
    }

    /**//// 对字符串的特化处理
    InBuffer& operator >> (char*& value)
    {
        const char* str = (const char*)_buf + _pos;
        while ((size_t)(str - (const char*)_buf) < _bufSize && *str++);
        size_t bytes = (size_t)(str - (char*)_buf) - _pos;
        if (bytes > 0 && _pos + bytes <= _bufSize)
        {
            if (*((char*)_buf + _pos + bytes - 1) != 0) // 不是0结尾
            {
                setstate(Buffer::_eof|Buffer::_bad);
                return (*this);
            }
            value = (char*)_buf + _pos;
            _pos += bytes;

            if (_pos == _bufSize)
                setstate(Buffer::_eof);
        }
        else
        {
            setstate(Buffer::_eof|Buffer::_fail);
            value = 0;
        }

        return (*this);
    }

    /**//// 对常宽字符串的特化处理
    InBuffer& operator >> (const wchar_t*& value)
    {
        const wchar_t* str = (const wchar_t*)((int8*)_buf + _pos);
        while ((size_t)((int8*)str - (int8*)_buf) < _bufSize && *str++);
        size_t bytes = (size_t)((int8*)str - (int8*)_buf) - _pos;
        if (bytes > 0 && _pos + bytes <= _bufSize)
        {
            if (*(const wchar_t*)((int8*)_buf + _pos + bytes - sizeof(wchar_t)) != 0) // 不是0结尾
            {
                setstate(Buffer::_eof|Buffer::_bad);
                return (*this);
            }
            value = (const wchar_t*)((int8*)_buf + _pos);
            _pos += bytes;

            if (_pos == _bufSize)
                setstate(Buffer::_eof);
        }
        else
        {
            setstate(Buffer::_eof|Buffer::_fail);
            value = 0;
        }

        return (*this);
    }

    /**//// 对宽字符串的特化处理
    InBuffer& operator >> (wchar_t*& value)
    {
        const wchar_t* str = (const wchar_t*)((int8*)_buf + _pos);
        while ((size_t)((int8*)str - (int8*)_buf) < _bufSize && *str++);
        size_t bytes = (size_t)((int8*)str - (int8*)_buf) - _pos;
        if (bytes > 0 && _pos + bytes <= _bufSize)
        {
            if (*(const wchar_t*)((int8*)_buf + _pos + bytes - sizeof(wchar_t)) != 0) // 不是0结尾
            {
                setstate(Buffer::_eof|Buffer::_bad);
                return (*this);
            }
            value = (wchar_t*)((int8*)_buf + _pos);
            _pos += bytes;

            if (_pos == _bufSize)
                setstate(Buffer::_eof);
        }
        else
        {
            setstate(Buffer::_eof|Buffer::_fail);
            value = 0;
        }

        return (*this);
    }


    /**//// 跳过某种数据类型（不进行赋值，仅仅改变当前位置）
    template <class T> InBuffer& skip()
    {
        if (_pos + sizeof(T) <= _bufSize)
        {
            _pos += sizeof(T);

            if (_pos == _bufSize)
                setstate(Buffer::_eof);
        }
        else
            setstate(Buffer::_eof|Buffer::_fail);
        return (*this);
    }

    /**//// 在当前位置导出一段缓冲
    InBuffer& pop(void* buffer, size_t bytes)
    {
        if (buffer == 0 || bytes == 0)
        {
            setstate(Buffer::_bad|Buffer::_fail);
            return (*this);
        }

        if (_pos + bytes <= _bufSize)
        {
            memcpy(buffer, (char*)_buf + _pos, bytes);
            _pos += bytes;

            if (_pos == _bufSize)
                setstate(Buffer::_eof);
        }
        else
        {
            setstate(Buffer::_eof|Buffer::_fail);
        }
        return (*this);
    }
};

}