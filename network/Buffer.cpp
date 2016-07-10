#include "Buffer.h"
#include <assert.h>

namespace Network
{

void Buffer::clear()
{
    memset(_buf, 0, _bufSize);
    _pos = 0;
    _state = _good;
}

void Buffer::skipn( size_t bytes )
{
    if (_pos + bytes <= _bufSize)
    {
        _pos += bytes;

        if (_pos == _bufSize)
            setstate(Buffer::_eof);
    }
    else
        setstate(Buffer::_eof|Buffer::_fail);
}


size_t OutBuffer::size() const
{
    return pos();
}
    
//将缓冲区前面bytes大小的数据移除掉
void OutBuffer::shrink(size_t bytes)
{
    if (bytes == 0)
    {
        return;
    }

    if (bytes > capacity())
    {
        bytes = capacity();
    }

    if (bytes >= size())
    {
        clear();//_pos = 0
    }
    else
    {
        size_t remain = size() - bytes;
        memmove(_buf, (char*)_buf + bytes, remain);
        _pos = remain;
    }
}


}