#pragma once

#include "arc/core/numeric_types.hpp"
#include "arc/string/all.hpp"

namespace arc
{

    class buffer_writer
    {
    public:
        buffer_writer();
        buffer_writer(void* ptr, uint32 n);
    public:
        void init(void* ptr, uint32 n);
    public:
        bool write(const char* s);
		bool write(const unsigned char* s);
        bool write(const char* ptr, uint32 n);
        bool write(char c);
        bool write(arc::String& s);
        bool write(arc::StringView s);

        template <uint32 N>
        bool write( const char ( &s )[N] )
        {
            return write(&s[0],N-1);
        }

    public:
        bool write(bool b);
        bool write(uint32 i);
        bool write(int32 i);

        bool write(uint64 i);
        bool write(int64 i);

        bool write(void* ptr);

    public:
        char* str() const;

    public:
        uint32 remaining() const;
        uint32 capacity() const;
        uint32 length() const;
        uint32 seek(uint32 pos);

    private:
        uint32 _iter = 0;
        uint32 _cap = 0;
        char* _ptr = nullptr;
    };

}
