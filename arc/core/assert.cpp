
#include "assert.hpp"

#ifdef ARC_DEBUG_ASSERT
    namespace arc
    {
        namespace assert
        {
			ARC_THREAD_LOCAL scope* _g_current_scope = nullptr;

            /// source: http://www.itshouldjustworktm.com/?p=376
            inline void interrupt_raise( const char* file_name, const char* function_name, int line_number, const char* cond, const char* fmt, va_list args )
            {
                printf( "\n");
                printf( "*************************************************************** \n" );
                printf( "*** ARC ASSERTION FAILURE \n");
                printf( "*************************************************************** \n" );
                printf( "*** Message:    " ); vprintf( fmt, args ); printf("\n");
                printf( "*** Expression: %s\n", cond);
                printf( "*** Scope:      "); print_scope(_g_current_scope); printf("\n");
                printf( "*** File:       %s\n",file_name);
                printf( "*** Line:       %d\n",line_number);
                printf( "*** Function:   %s\n",function_name);
                printf( "*************************************************************** \n" );
                printf( "*** Press return key to continue \n" );
                printf( "*************************************************************** " );
                printf( "\n");
                getchar();
            }

            void (*handler_function)( const char*, const char*, int, const char*, const char* fmt, va_list args ) = &interrupt_raise;
        }
    }
#endif
