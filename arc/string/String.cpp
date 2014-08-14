#include "String.hpp"

#include <cstring>
#include <atomic>

#include "arc/core.hpp"
#include "arc/string/all.hpp"
#include "arc/memory/Allocator.hpp"

namespace arc
{
    namespace string_implementation
    {
        struct header
        {
            std::atomic<int32>  ref_count;
            uint32              length;
        };
    }

    namespace asi = arc::string_implementation;

    static memory::Mallocator _g_string_allocator;

    String::String()
    {}

    String::String(const char *s, uint32 n)
    {
        if (n == 0) return;

        auto HEADER_SIZE = sizeof(asi::header);
        _data = (asi::header*)_g_string_allocator.allocate(HEADER_SIZE+n+1, alignof(asi::header));
        _data->ref_count = 1;
        _data->length = n;
        std::memcpy(str_data(),s,n);
        str_data()[n] = '\0';
    }

    String::String(const StringView other)
        : String(other.c_str(),other.length())
    {}

    String::~String()
    {
        if (_data == nullptr) return;

    }

    String::String(const String &other)
    {
        other.increase_ref_count();
        _data = other._data;
    }

    String &String::operator=(const String &other)
    {
        other.increase_ref_count();
        this->decrease_ref_count();
        _data = other._data;
        return *this;
    }

    String::String(String&& other)
    {
        _data = other._data;
        other._data = nullptr;
    }

    String& String::operator= (String&& other)
    {
        _data = other._data;
        other._data = nullptr;
        return *this;
    }

    const char *String::c_str() const
    {
        return _data == nullptr ? "" : str_data();
    }

    uint32 String::length() const
    {
        return _data == nullptr ? 0 : _data->length;
    }

    bool String::operator ==(const String &other) const
    {
        if (_data == other._data)
            return true;
        if (_data == nullptr || other._data == nullptr)
            return false;
        return 0 == strcmp(c_str(),other.c_str());
    }

    String String::_Make_Raw(uint32 n)
    {
        String str;
        if (n == 0) return str;

        auto HEADER_SIZE = sizeof(asi::header);
        str._data = (asi::header*)_g_string_allocator.allocate(HEADER_SIZE+n+1,alignof(asi::header));
        str._data->ref_count = 1;
        str._data->length = n;
        str.str_data()[n] = '\0';

        return str;
    }

    void String::increase_ref_count() const
    {
        if (_data != nullptr) _data->ref_count += 1;
    }

    void String::decrease_ref_count()
    {
        if (_data != nullptr)
        {
            _data->ref_count -= 1;
            ARC_ASSERT(_data->ref_count >= 0, "String: invalid ref-count.");
            if (_data->ref_count == 0)
            {
                _g_string_allocator.free(_data);
                _data = nullptr;
            }
        }
    }

    char *String::str_data() const
    {
        char* p = (char*)_data;
        p += sizeof(asi::header);
        return p;
    }

} // namespace arc
