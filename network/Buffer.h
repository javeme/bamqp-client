#pragma once
#include <assert.h>
#include <string.h>
#include <wchar.h>

namespace Network
{

typedef unsigned char int8;

// ����������(http://www.cppblog.com/PeakGao/archive/2007/06/07/25709.html)
class Buffer
{
public:
    enum bufstate{ _good = 0x0, _eof = 0x1, _fail = 0x2, _bad = 0x4/**//*���ش���*/};

protected:
    void*        _buf;        /**//// �������׵�ַ
    size_t       _bufSize;    /**//// ��������С
    size_t       _pos;        /**//// ��ǰ����λ��
    bufstate     _state;      /**//// ��������ǰ������״̬

    // ���죨������
protected:
    Buffer() : _buf(0), _bufSize(0), _pos(0), _state(_good) { }

public:
    Buffer(void* buffer, size_t maxsize) : _buf(buffer), _bufSize(maxsize), _pos(0), _state(_good)
    {
        assert(buffer != 0 && maxsize > 0);
    }

    virtual ~Buffer() { }

    // ״̬��أ����ڼ������Ľ����
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


    // ���Լ�����
public:
    /**//// �������������
    virtual void clear();

    /**//// ����ǰλ������ƶ�ָ���Ĵ�С
    virtual void skipn(size_t bytes);

    /**//// ��ȡ��������ַ
    virtual const char* data() const
    {
        return (char*)_buf;
    }

    /**//// ��ȡ��������ǰλ�õĵ�ַ
    virtual const char* current() const
    {
        return (char*)_buf + _pos;
    }

    /**//// ��ȡ���������ݲ����ĵ�ǰ(��/д)λ��ƫ��
    virtual size_t pos() const
    {
        return _pos;
    }

    /**//// ��ȡ������ʣ��δ����(��/д)��С
    virtual size_t remains() const
    {
        return capacity() - pos();
    }

    /**//// ��ȡ�����������������������Ĵ�С��
    virtual size_t capacity() const
    {
        return _bufSize;
    }

    /**//// ��ȡ�����������ݵ�ʵ��ռ�óߴ�
    virtual size_t size() const = 0;

    //��������ǰ��bytes��С�������Ƴ���
    virtual void shrink(size_t bytes) = 0;
};




/**//// ������壨ָ�������ݵ��뵽�������У�
class OutBuffer : public Buffer
{
    // ���죨������
protected:
    OutBuffer() : Buffer() {}

public:
    OutBuffer(void* buffer, size_t maxsize) : Buffer(buffer, maxsize) {}

    virtual ~OutBuffer() {}


    // ����
public:
    /**//// ��ȡ�����������ݵ�ʵ��ռ�óߴ�
    size_t size() const;

    //��������ǰ��bytes��С�������Ƴ���
    virtual void shrink(size_t bytes);

    /**//// �򻺳���д����־��й̶����ȵ��������ͣ����������ͺ͸������ͣ��ṹ��
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
    
    /**//// �Գ��ַ������ػ�����
    OutBuffer& operator << (const char* value)
    {
        return push((void*)value, strlen(value) + sizeof(char));
    }
    
    /**//// ���ַ������ػ�����
    OutBuffer& operator << (char* value)
    {
        return push((void*)value, strlen(value) + sizeof(char));
    }

    /**//// �Գ����ַ������ػ�����
    OutBuffer& operator << (const wchar_t* value)
    {
        return push((void*)value, (wcslen(value) + 1) * sizeof(wchar_t));
    }

    /**//// �Կ��ַ������ػ�����
    OutBuffer& operator << (wchar_t* value)
    {
        return push((void*)value, (wcslen(value) + 1) * sizeof(wchar_t));
    }


    /**//// ����ĳ���������ͣ������и�ֵ�������ı䵱ǰλ�ã�
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

    /**//// �ڵ�ǰλ�õ���һ�λ���
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


/**//// �̶���С���������
template <size_t _MaxBytes = 256>
class FixOutBuffer : public OutBuffer
{
protected:
    enum {_Buf_Size = _MaxBytes ? ((_MaxBytes + 7) & ~7) : 8}; // 8�ֽڶ���
    char _data[_Buf_Size];

public:
    FixOutBuffer() : OutBuffer(_data, _Buf_Size){}
    virtual ~FixOutBuffer() {}
};




/**//// ���뻺�壨ָ�ӻ������е����������ݣ�
class InBuffer : public Buffer
{
    // ���죨������
protected:
    InBuffer() : Buffer() { }

public:
    InBuffer(void* buffer, size_t actualSize) : Buffer(buffer, actualSize)    { }

    virtual ~InBuffer() { }


    // ����
public:
    /**//// ��ȡ�����������ݵ�ʵ��ռ�óߴ�
    size_t size() const
    {
        return _bufSize;
    }

    virtual void shrink(size_t bytes)
    {
        return;
    }

    /**//// �ӻ������������־��й̶����ȵ��������ͣ����������ͺ͸������ͣ��ṹ��
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

    //ʹ��GCC����ʱ,�öδ������Ҫ��ֲ��buffer.cpp�ļ��з������
    /**//// �Գ��ַ������ػ�����
    InBuffer& operator >> (const char*& value)
    {
        const char* str = (const char*)_buf + _pos;
        while ((size_t)(str - (const char*)_buf) < _bufSize && *str++);
        size_t bytes = (size_t)(str - (char*)_buf) - _pos;
        if (bytes > 0 && _pos + bytes <= _bufSize)
        {
            if (*((char*)_buf + _pos + bytes - 1) != 0) // ����0��β���ַ���
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

    /**//// ���ַ������ػ�����
    InBuffer& operator >> (char*& value)
    {
        const char* str = (const char*)_buf + _pos;
        while ((size_t)(str - (const char*)_buf) < _bufSize && *str++);
        size_t bytes = (size_t)(str - (char*)_buf) - _pos;
        if (bytes > 0 && _pos + bytes <= _bufSize)
        {
            if (*((char*)_buf + _pos + bytes - 1) != 0) // ����0��β
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

    /**//// �Գ����ַ������ػ�����
    InBuffer& operator >> (const wchar_t*& value)
    {
        const wchar_t* str = (const wchar_t*)((int8*)_buf + _pos);
        while ((size_t)((int8*)str - (int8*)_buf) < _bufSize && *str++);
        size_t bytes = (size_t)((int8*)str - (int8*)_buf) - _pos;
        if (bytes > 0 && _pos + bytes <= _bufSize)
        {
            if (*(const wchar_t*)((int8*)_buf + _pos + bytes - sizeof(wchar_t)) != 0) // ����0��β
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

    /**//// �Կ��ַ������ػ�����
    InBuffer& operator >> (wchar_t*& value)
    {
        const wchar_t* str = (const wchar_t*)((int8*)_buf + _pos);
        while ((size_t)((int8*)str - (int8*)_buf) < _bufSize && *str++);
        size_t bytes = (size_t)((int8*)str - (int8*)_buf) - _pos;
        if (bytes > 0 && _pos + bytes <= _bufSize)
        {
            if (*(const wchar_t*)((int8*)_buf + _pos + bytes - sizeof(wchar_t)) != 0) // ����0��β
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


    /**//// ����ĳ���������ͣ������и�ֵ�������ı䵱ǰλ�ã�
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

    /**//// �ڵ�ǰλ�õ���һ�λ���
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