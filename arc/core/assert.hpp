#pragma once

#include <cstdarg>
#include <cstdio>
#include <exception>

#include "compatibility.hpp"

#define ARC_DEBUG_ASSERT

#ifdef ARC_DEBUG_ASSERT

    namespace arc
    {
        namespace assert
        {
            struct scope;

            extern ARC_THREAD_LOCAL scope* _g_current_scope;

            struct scope
            {
                inline scope(const char* name) : _name(name)
                {
                    _prev = _g_current_scope;
                    _g_current_scope = this;
                }

                inline ~scope()
                {
                    _g_current_scope = _prev;
                }

                scope*        _prev;
                const char*   _name;
            };

            inline void print_scope(scope* s)
            {
                if (s == nullptr) return;
                print_scope(s->_prev);
                printf("::%s",s->_name);
            }

            extern void (*handler_function)( const char*, const char*, int, const char*, const char* fmt, va_list args );

            inline void raise( const char* file_name, const char* function_name, int line_number, const char* cond, const char* fmt, ... )
            {
                va_list args;
                va_start( args, fmt );

                if (handler_function != nullptr)
                {
                    handler_function(file_name,function_name,line_number,cond,fmt,args);
                }

                va_end( args );
            }

        }
    }

    #define ARC_ASSERT_SCOPE(name) \
        const arc::assert::scope __arc_assert_scope(name);

    #define ARC_ASSERT(condition,format,...) \
        {if (!(condition)) arc::assert::raise(__FILE__,__FUNCTION__,__LINE__,#condition,format,##__VA_ARGS__); }

    #define ARC_ASSERT_FWD(condition,file, function, line, format,...) \
        {if (!(condition)) arc::assert::raise(file,function,line,#condition,format,##__VA_ARGS__); }

    #define ARC_NOT_IMPLEMENTED                                 \
        ARC_ASSERT(false,"not implemented.")                    \
        throw std::exception();                                 \

#endif
