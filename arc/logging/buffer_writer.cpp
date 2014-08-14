#include "buffer_writer.hpp"

#include "arc/core/assert.hpp"

#include <cstring>
#include <cstdio>

#ifdef _WIN32
#define snprintf(a,b,c,d) _snprintf_s(a,b,b,c,d);
#endif

namespace arc
{

    buffer_writer::buffer_writer()
        :  _cap(0),_ptr(nullptr)
    {}

    buffer_writer::buffer_writer(void *ptr, uint32 n)
        : _cap(n-1),_ptr((char*)ptr)
    {
        _ptr[_cap] = '\0';
    }

    void buffer_writer::init(void *ptr, uint32 n)
    {
        _ptr = (char*)ptr;
        _cap = n-1;
        _iter = 0;
    }

	bool buffer_writer::write(const unsigned char* s)
	{
		return write((const char*)s);
	}

    bool buffer_writer::write(const char * s)
    {
        if (_ptr == nullptr) return false;

        auto r = remaining();
        auto cntr = snprintf( _ptr+_iter, r+1, "%s", s );
        ARC_ASSERT(cntr >= 0, "snprintf error");
        uint32 cnt = cntr < 0 ? 0 : cntr;
        if (cnt <= r)
        {
            _iter += cnt;
            return true;
        }
        else
        {
            _iter = _cap;
            return false;
        }
    }

    bool buffer_writer::write(const char * ptr, uint32 n)
    {
        bool allWritten = true;
        if (n > remaining())
        {
            allWritten = false;
            n = remaining();
        }

        std::memcpy(_ptr+_iter, ptr, n*sizeof(char));
        _iter += n;

        return allWritten;
    }

    bool buffer_writer::write(arc::StringView s)
    {
        return write(s.c_str(),s.length());
    }

    bool buffer_writer::write(char c)
    {
        if (remaining() == 0)  return false;

        _ptr[_iter] = c;
        _iter += 1;
        return true;
    }

    uint32 buffer_writer::remaining() const
    {
        return _cap - _iter;
    }

    uint32 buffer_writer::length() const
    {
        return _iter;
    }

    uint32 buffer_writer::seek(uint32 pos)
    {
        if (pos > _cap) pos = _cap;
        _iter = pos;
        return _iter;
    }

    uint32 buffer_writer::capacity() const
    {
        return _cap;
    }

    char *buffer_writer::str() const
    {
        if (_ptr != nullptr) _ptr[_iter] = '\0';
        return _ptr;
    }

    bool buffer_writer::write(bool b)
    {
        if (b)
            return write("true");
        else
            return write("false");
    }

    bool buffer_writer::write(uint32 i)
    {
        if (_ptr == nullptr) return false;

        auto r = remaining();
        auto cntr = snprintf( _ptr+_iter, r+1, "%u", i );
        ARC_ASSERT(cntr >= 0, "snprintf error");
        uint32 cnt = cntr < 0 ? 0 : cntr;
        if (cnt <= r)
        {
            _iter += cnt;
            return true;
        }
        else
        {
            _iter = _cap;
            return false;
        }
    }

    bool buffer_writer::write(int32 i)
    {
        if (_ptr == nullptr) return false;

        auto r = remaining();
        auto cntr = snprintf( _ptr+_iter, r+1, "%i", i );
        ARC_ASSERT(cntr >= 0, "snprintf error");
        uint32 cnt = cntr < 0 ? 0 : cntr;
        if (cnt <= r)
        {
            _iter += cnt;
            return true;
        }
        else
        {
            _iter = _cap;
            return false;
        }
    }

    bool buffer_writer::write(void* ptr)
    {
        if (_ptr == nullptr) return false;

        if (ptr == nullptr) return write("0x0",3);

        auto r = remaining();
        auto cntr = snprintf( _ptr+_iter, r+1, "%p", ptr );
        ARC_ASSERT(cntr >= 0, "snprintf error");
        uint32 cnt = cntr < 0 ? 0 : cntr;
        if (cnt <= r)
        {
            _iter += cnt;
            return true;
        }
        else
        {
            _iter = _cap;
            return false;
        }
    }

    bool buffer_writer::write(uint64 i)
    {
        if (_ptr == nullptr) return false;

        auto r = remaining();
        auto cntr = snprintf( _ptr+_iter, r+1, "%lu", i );
        ARC_ASSERT(cntr >= 0, "snprintf error");
        uint32 cnt = cntr < 0 ? 0 : cntr;
        if (cnt <= r)
        {
            _iter += cnt;
            return true;
        }
        else
        {
            _iter = _cap;
            return false;
        }
    }

    bool buffer_writer::write(int64 i)
    {
        if (_ptr == nullptr) return false;

        auto r = remaining();
        auto cntr = snprintf( _ptr+_iter, r+1, "%li", i );
        ARC_ASSERT(cntr >= 0, "snprintf error");
        uint32 cnt = cntr < 0 ? 0 : cntr;
        if (cnt <= r)
        {
            _iter += cnt;
            return true;
        }
        else
        {
            _iter = _cap;
            return false;
        }
    }

    bool buffer_writer::write(arc::String& s)
    {
        return write(s.c_str(),s.length());
    }

}
