#pragma once

#include "arc/core/numeric_types.hpp"

namespace arc
{
    namespace string_implementation { struct header; }
    class StringView;

	/* A heap allocated String that uses reference counting. */
    class String
    {
    public:
        String();
        String(const String& other);
        String(String&& other);
        String(const char* s, uint32 n);
        String(const StringView other);

        template<uint32 N>
        explicit String( const char ( &s )[N] ) : String(s,N-1) {}

    public:
        ~String();
        String& operator=(const String& other);
        String& operator=(String&& other);

    public:
        /// the raw, null-terminated c-string
        const char* c_str() const;
        uint32 length() const;

    public:
        bool operator==(const String& other) const;

    public:
        static String _Make_Raw(uint32 n);

    private:
        void increase_ref_count() const;
        void decrease_ref_count();
        char* str_data() const;

    private:
         mutable string_implementation::header* _data = nullptr;
    };

} // namespace arc
