#include "util.hpp"

#include <cstring>
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
