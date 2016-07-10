/**
 *  IOCP in/out Buffer
 */
#pragma once
#include "../amqpcpp/src/includes.h"

namespace AMQP {

/**
 *  Class definition
 */
class InputBuffer : public ByteBuffer
{
public:
    /**
     *  Constructor
     *  Note that we pass 0 to the constructor because the buffer seems to be empty
     *  @param  size        initial size to allocated
     */
    InputBuffer(size_t size) : ByteBuffer((char *)malloc(size), 0), _capacity(size) {}
    
    /**
     *  No copy'ing
     *  @param  that        object to copy
     */
    InputBuffer(const InputBuffer &that) METHOD_DELETE;
    
    /**
     *  Move constructor
     *  @param  that
     */
    InputBuffer(InputBuffer &&that) : ByteBuffer(std::move(that)) {}
    
    /**
     *  Destructor
     */
    virtual ~InputBuffer()
    {
        // free memory
        if (_data) free((void *)_data);
    }

    /**
     *  Move assignment operator
     *  @param  that
     */
    InputBuffer &operator=(InputBuffer &&that)
    {
        // skip self-assignment
        if (this == &that) return *this;
        
        // call base
        ByteBuffer::operator=(std::move(that));
        
        // done
        return *this;
    }
    
    /**
     *  Reallocate date
     *  @param  size
     */
    void reallocate(size_t size)
    {
        // update data
        _data = (char *)realloc((void *)_data, size);
        _capacity = size;
   }
    
    /**
    *  Received data
    *  @param data
    *  @param len
    */
    void received(unsigned char* data, size_t len)
    {
        size_t need = _size + len;
        if(need > _capacity)
            reallocate(need * 2);
        memcpy((void*)(_data + _size), data, len);
        _size += len;
    }
    
    void shrink(size_t len)
    {
        if(len >= _size) {
            _size = 0;
        }
        else {
            memmove((void*)_data, _data + len, _size - len);
            _size -= len;
        }
    }
protected:
    size_t _capacity;
};

    
/**
 *  Class definition
 */
class OutputBuffer : public Buffer
{
public:
    /**
     *  Regular constructor
     */
    OutputBuffer() : _size(0), _skip(0) {}
    
    /**
     *  No copy'ing allowed
     *  @param  that
     */
    OutputBuffer(const OutputBuffer &that) METHOD_DELETE;

    /**
     *  Move operator
     *  @param  that
     */
    OutputBuffer(OutputBuffer &&that) : 
        _buffers(std::move(that._buffers)), 
        _skip(that._skip), 
        _size(that._size)
    {
        // reset other object
        that._skip = 0;
        that._size = 0;
    }
    
    /**
     *  Move assignment operator
     *  @param  that
     */
    OutputBuffer &operator=(OutputBuffer &&that)
    {
        // skip self-assignment
        if (this == &that) return *this;
        
        // swap buffers
        _buffers.swap(that._buffers);
        
        // swap integers
        std::swap(_skip, that._skip);
        std::swap(_size, that._size);
        
        // done
        return *this;
    }
    
    /**
     *  Does the buffer exist (is it non-empty)
     *  @return bool
     */
    operator bool () const
    {
        // there must be a size
        return _size > 0;
    }
    
    /**
     *  Is the buffer empty
     *  @return bool
     */
    bool operator!() const
    {
        // size should be zero
        return _size == 0;
    }

    /**
     *  Total size of the buffer
     *  @return size_t
     */
    size_t size() const
    {
        // this simply is a member
        return _size;
    }

    /**
     *  Add data to the buffer
     *  @param  buffer
     *  @param  size
     */
    void add(const char *buffer, size_t size)
    {
        // add element
        _buffers.emplace_back(std::vector<char>(buffer, buffer + size));
    
        // update total size
        _size += size;
    }
    
    /**
     *  Shrink the buffer with a number of bytes
     *  @param  toremove
     */
    void shrink(size_t toremove)
    {
        // are we removing everything?
        if (toremove >= _size)
        {
            // reset all
            _buffers.clear(); 
            _skip = _size = 0;
        }
        else
        {
            // keep looping
            while (toremove > 0)
            {
                // access to the first buffer
                const auto &first = _buffers.front();
                
                // actual used bytes in first buffer
                size_t bytes = first.size() - _skip;
                
                // can we remove the first buffer completely?
                if (toremove >= bytes)
                {
                    // we're going to remove the first item, update sizes
                    _size -= bytes;
                    _skip = 0;
                    
                    // number of bytes that still have to be removed
                    toremove -= bytes;
                    
                    // remove first buffer
                    _buffers.pop_front();
                }
                else
                {
                    // we should remove the first buffer partially
                    _skip += toremove;
                    _size -= toremove;
                    
                    // done
                    toremove = 0;
                }
            }
        }
    }
    
    /**
     *  Buf 
     */
    struct _Buf {
        const unsigned char *data;
        size_t size;
    };
    
    /**
     *  Get the first buffer
     *  @return ssize_t
     */
    _Buf first()
    {
        const auto &str = _buffers.front();
        _Buf buf;
        buf.data = (const unsigned char *)(str.data() + _skip);
        buf.size = str.size() - _skip;
        return buf;
    }

    virtual char byte(size_t pos) const
    {
        throw std::exception("The method or operation is not implemented.");
    }

    virtual const char * data(size_t pos, size_t size) const
    {
        throw std::exception("The method or operation is not implemented.");
    }

    virtual void * copy(size_t pos, size_t size, void *buffer) const
    {
        throw std::exception("The method or operation is not implemented.");
    }

protected:
    /**
     *  All output buffers
     *  @var std::deque
     */
    mutable std::deque<std::vector<char>> _buffers;

    /**
     *  Number of bytes in first buffer that is no longer in use
     *  @var size_t
     */
    size_t _skip;
    
    /**
     *  Total number of bytes in the buffer
     *  @var size_t
     */
    size_t _size;
};


/**
 *  End of namespace
 */
}

