/****************************************************************************
    Copyright (C) 2013 Adrian Blumer (blumer.adrian@gmail.com)

    All Rights Reserved.

    You may use, distribute and modify this code under the terms of the
    MIT license (http://opensource.org/licenses/MIT).
*****************************************************************************/

#pragma once

#include "arc/core.hpp"

#define ARC_DEBUG_GL_ERROR_CHECKING

#if defined(DEBUG) || defined(ARC_DEBUG_GL_ERROR_CHECKING)
#	define ARC_GL_CHECK_FOR_ERRORS()	arc::gl::check_errors(__FILE__,__FUNCTION__,__LINE__)
#   define ARC_GL_CLEAR_ERRORS()        arc::gl::clear_errors()
#else
#	define ARC_GL_CHECK_FOR_ERRORS()
#   define ARC_GL_CLEAR_ERRORS()
#endif

namespace arc
{
namespace gl
{

inline void clear_errors()
{
    while( glGetError() != GL_NO_ERROR );
}

inline void check_errors(const char* file_name, const char* function_name, int line_number)
{
    const char * errorString = nullptr;
    GLuint errorCode = glGetError();

    switch( errorCode )
    {
    case GL_NO_ERROR:
        return;

    case GL_INVALID_ENUM:
        errorString = "GL_INVALID_ENUM";
        break;

    case GL_INVALID_VALUE:
        errorString = "GL_INVALID_VALUE";
        break;

    case GL_INVALID_OPERATION:
        errorString = "GL_INVALID_OPERATION";
        break;

    case GL_INVALID_FRAMEBUFFER_OPERATION:
        errorString = "GL_INVALID_FRAMEBUFFER_OPERATION";
        break;

    case GL_OUT_OF_MEMORY:
        errorString = "GL_OUT_OF_MEMORY";
        break;

    default:
        errorString = "UNKNOWN";
        break;
    }
    ARC_ASSERT_FWD(false,file_name,function_name,line_number,"OpenGL error: %u (%s)",errorCode,errorString);
}

}} // namespace arc::gl
