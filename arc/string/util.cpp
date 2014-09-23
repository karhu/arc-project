#include "util.hpp"

#include <cstring>
#include <stdio.h>

#include <algorithm>

#include "arc/core/assert.hpp"

#include "all.hpp"

namespace arc
{
    bool operator==(const String& first, const StringView& second)
    {
        if (first.length() != second.length()) return false;
        return 0 == strncmp(first.c_str(),second.c_str(),std::min(first.length(),second.length()));
    }

    bool operator==(const StringView& first, const String& second)
    {
        return second == first;
    }

    bool operator ==(const StringView &first, const StringView &second)
    {
        if (first.length() != second.length()) return false;
        return 0 == strncmp(first.c_str(),second.c_str(),std::min(first.length(),second.length()));
    }


} // namespace arc

namespace arc { namespace cstr {

	uint32 length(const char* str, uint32 max_length)
	{
		return strnlen(str, max_length);
	}

}}

namespace arc { namespace string {

	StringView remove_trailing(StringView str, bool(*filter)(char))
	{
		int32 n = str.length();
		while (n > 0 && filter(str.c_str()[n - 1]))
		{
			n -= 1;
		}
		return StringView(str, 0, n);
	}

	bool is_whitespace(char c)
	{
		switch (c)
		{
		case ' ':
		case '\t':
		case '\n':
			return true;
		default:
			return false;
		}
	}

}} 