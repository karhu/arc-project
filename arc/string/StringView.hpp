#pragma once

#include "arc/core/numeric_types.hpp"
#include "arc/collections/Slice.hpp"

namespace arc
{
    class String;

    class StringView
    {
    public: // default constructors & assignment operators
        StringView()                                    = default;
        StringView(const StringView& other)             = default;
        StringView& operator=(const StringView& other)  = default;
		StringView(StringView&& other);
		StringView& operator=(StringView&& other);

    public: // other constructors
        StringView(const StringView& str, uint32 begin, uint32 end);
        StringView(const char* str, uint32 begin, uint32 end);
        StringView(const String& str);
        StringView(const String& str, uint32 begin, uint32 end);

    public: // constructor from string literals

#ifdef _WIN32 // win32 & win64
		template<uint32 N>
		StringView( const char ( &s )[N] ) : _str(s) , _length(N-1) {}
#else // assume full c++11 support
		template<uint32 N>
		constexpr StringView(const char(&s)[N]) : _str(s), _length(N - 1) {}
#endif

    public:
        /// this string might not be null terminated!
        const char* c_str() const;
        uint32 length() const;

    private:
        const char* _str = nullptr;
        uint32 _length = 0;
    };

	bool write_string(Slice<char>& buffer, StringView v);

} // namespace arc
