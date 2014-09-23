#include "StringView.hpp"

#include <cstring>

#include "arc/core/assert.hpp"
#include "arc/string/all.hpp"

namespace arc
{
	StringView::StringView(StringView&& other)
		: _str(other._str)
		, _length(other._length)
	{}

	StringView& StringView::operator=(StringView&& other)
	{
		_str = other._str;
		_length = other._length;
		return *this;
	}

    const char *StringView::c_str() const
    {
        return _str;
    }

    uint32 StringView::length() const
    {
        return _length;
    }

    StringView::StringView(const StringView &str, uint32 begin, uint32 end)
        : _str(str.c_str()+begin)
        , _length(end-begin)
    {
        ARC_ASSERT(begin <= str.length(),"Invalid StringView begin.");
        ARC_ASSERT(end <= str.length(), "Invalid StringView end");
    }

    StringView::StringView(const String &str)
        : _str(str.c_str())
        , _length(str.length())
    {}

    StringView::StringView(const char *str, uint32 begin, uint32 end)
        : _str(str+begin)
        , _length(end-begin)
    {}

	bool write_string(Slice<char>& buffer, StringView v)
	{
		if (buffer.size() < v.length()) return false;
		
		memcpy(buffer.ptr(), v.c_str(), v.length());
		buffer.trim_front(v.length());
		return true;
	}

} // namespace arc
