#pragma once


#include "arc/core.hpp"

#include "arc/string/String.hpp"
#include "arc/string/StringView.hpp"

namespace arc { namespace hash {

    namespace fnv_1a {

        // FNV-1a constants
        static const uint64 BASIS = 14695981039346656037ULL;
        static const uint64 PRIME = 1099511628211ULL;

        // compile-time hash helper function
        inline ARC_CONSTEXPR uint64 _hash_one(char c, const char* remain, uint64 value)
        {
            return c == 0 ? value : _hash_one(remain[0], remain + 1, (value ^ c) * PRIME);
        }

        // run-time hash 64bit
        inline uint64 rt64(const char* str)
        {
            uint64 hash = BASIS;
            while (*str != 0) {
                hash ^= str[0];
                hash *= PRIME;
                ++str;
            }
            return hash;
        }

        // run-time hash 64bit
        inline uint64 rt64(const char* str, uint32_t len)
        {
            uint64 hash = BASIS;
            for (uint32_t i=0; i<len; i++)
            {
                hash ^= str[i];
                hash *= PRIME;
            }
            return hash;
        }

        // compile-time hash 64bit
		ARC_CONSTEXPR inline uint64 ct64(const char* str)
        {
            return _hash_one(str[0], str + 1, BASIS);
        }

    } // namespace fnv_1a

} // namespace hash

	// 64bit string hash /////////////////////////////////////////////////

	struct StringHash64
	{
		ARC_CONSTEXPR StringHash64() {}
		ARC_CONSTEXPR StringHash64(uint64 value) : m_value(value) {}

		uint64 value() { return m_value;  }
	private:
		uint64 m_value = 0;
	};

	template<uint32 N> inline
	ARC_CONSTEXPR StringHash64 string_hash64(const char(&str)[N])
	{
		return { hash::fnv_1a::ct64(str) };
	}

	inline StringHash64 string_hash64(const String& s)
	{
		return { hash::fnv_1a::rt64(s.c_str(), s.length()) };
	}

	inline StringHash64 string_hash64(StringView s)
	{
		return{ hash::fnv_1a::rt64(s.c_str(), s.length()) };
	}

	// 32bit string hash ///////////////////////////////////////////////////

	// default string hash /////////////////////////////////////////////////

	using StringHash = StringHash64;

	template<uint32 N> inline
	ARC_CONSTEXPR StringHash64 string_hash(const char(&str)[N])
	{
		return{ hash::fnv_1a::ct64(str) };
	}

	inline StringHash64 string_hash(const String& s)
	{
		return{ hash::fnv_1a::rt64(s.c_str(), s.length()) };
	}

	inline StringHash64 string_hash(StringView s)
	{
		return{ hash::fnv_1a::rt64(s.c_str(), s.length()) };
	}

} // namespace arc

#define SH(ARG) string_hash(""ARG)