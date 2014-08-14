#pragma once

#include "arc/core.hpp"

#define GLEW_STATIC
#include "GL/glew.h"

#ifdef _WIN32 // win32 & win64
	#include <GL/gl.h>
#else 
	#include <GL/glcorearb.h>
#endif


namespace arc { namespace gl {

    enum class Primitive: GLenum
    {
        Points = GL_POINTS,
        LineStrip = GL_LINE_STRIP,
        LineLoop = GL_LINE_LOOP,
        Lines = GL_LINES,
        LineStripAdjacency = GL_LINE_STRIP_ADJACENCY,
        LinesAdjacency = GL_LINES_ADJACENCY,
        TriangleStrip = GL_TRIANGLE_STRIP,
        TriangleFan = GL_TRIANGLE_FAN,
        Triangles = GL_TRIANGLES,
        TriangleStripAdjacency = GL_TRIANGLE_STRIP_ADJACENCY,
        TrianglesAdjacency = GL_TRIANGLES_ADJACENCY
    };

    enum class BufferType : GLenum
    {
        Array = GL_ARRAY_BUFFER,
        CopyRead = GL_COPY_READ_BUFFER,
        CopyWrite = GL_COPY_WRITE_BUFFER,
        ElementArray = GL_ELEMENT_ARRAY_BUFFER,
        PixelPack = GL_PIXEL_PACK_BUFFER,
        PixelUnpack = GL_PIXEL_UNPACK_BUFFER,
        Texture = GL_TEXTURE_BUFFER,
        TransformFeedback = GL_TRANSFORM_FEEDBACK_BUFFER,
        Uniform = GL_UNIFORM_BUFFER,
        DrawIndirect = GL_DRAW_INDIRECT_BUFFER
    };

    enum class ShaderBufferTarget : GLenum
    {
        AtomicCounter = GL_ATOMIC_COUNTER_BUFFER,
        TransformFeedback = GL_TRANSFORM_FEEDBACK_BUFFER,
        Uniform = GL_UNIFORM_BUFFER,
        ShaderStorage = GL_SHADER_STORAGE_BUFFER,
    };

    enum class BufferAccess : GLbitfield
    {
        Read = GL_MAP_READ_BIT,
        Write = GL_MAP_WRITE_BIT,
        InvalidateRange = GL_MAP_INVALIDATE_RANGE_BIT,
        InvalidateBuffer = GL_MAP_INVALIDATE_BUFFER_BIT,
        FlushExplicit = GL_MAP_FLUSH_EXPLICIT_BIT,
        Unsynchronized = GL_MAP_UNSYNCHRONIZED_BIT,
        Persistent = GL_MAP_PERSISTENT_BIT,
        Coherent = GL_MAP_COHERENT_BIT
    };

    inline BufferAccess operator|(BufferAccess a, BufferAccess b)
    {return static_cast<BufferAccess>(static_cast<GLbitfield>(a) | static_cast<GLbitfield>(b));}

    enum class BufferStorage: GLbitfield
    {
        DynamicStorage = GL_DYNAMIC_STORAGE_BIT,
        Read = GL_MAP_READ_BIT,
        Write = GL_MAP_WRITE_BIT,
        Persistent = GL_MAP_PERSISTENT_BIT,
        Coherent = GL_MAP_COHERENT_BIT,
        ClientStorageHint = GL_CLIENT_STORAGE_BIT
    };

    inline BufferStorage operator|(BufferStorage a, BufferStorage b)
    {return static_cast<BufferStorage>(static_cast<GLbitfield>(a) | static_cast<GLbitfield>(b));}


    enum class WaitSyncFlags : GLbitfield
    {
        FlushCommands = GL_SYNC_FLUSH_COMMANDS_BIT
    };

    inline WaitSyncFlags operator|(WaitSyncFlags a, WaitSyncFlags b)
    {return static_cast<WaitSyncFlags>(static_cast<int>(a) | static_cast<int>(b));}

    enum class WaitSyncResult : GLenum
    {
        AlreadySignaled = GL_ALREADY_SIGNALED,
        TimeoutExpired = GL_TIMEOUT_EXPIRED,
        ConditionSatisfied = GL_CONDITION_SATISFIED,
        WaitFailed = GL_WAIT_FAILED
    };

	enum class ClearBufferBits : GLbitfield
	{
		Color = GL_COLOR_BUFFER_BIT,
		Depth = GL_DEPTH_BUFFER_BIT,
		Accum = GL_ACCUM_BUFFER_BIT,
		Stencil = GL_STENCIL_BUFFER_BIT,
	};

	inline ClearBufferBits operator|(ClearBufferBits a, ClearBufferBits b)
	{
		return static_cast<ClearBufferBits>(static_cast<GLbitfield>(a) | static_cast<GLbitfield>(b));
	}

    enum class BufferUsage : GLenum
    {
        StreamDraw = GL_STREAM_DRAW,
        StreamRead = GL_STREAM_READ,
        StreamCopy = GL_STREAM_COPY,
        StaticDraw = GL_STATIC_DRAW,
        StaticRead = GL_STATIC_READ,
        StaticCopy = GL_STATIC_COPY,
        DynamicDraw = GL_DYNAMIC_DRAW,
        DynamicRead = GL_DYNAMIC_READ,
        DynamicCopy = GL_DYNAMIC_COPY
    };

    enum class ShaderType : GLenum
    {
        //Compute = GL_COMPUTE_SHADER,
        Vertex  = GL_VERTEX_SHADER,
        TesselationControl = GL_TESS_CONTROL_SHADER,
        TesselationEvaluation = GL_TESS_EVALUATION_SHADER,
        Geometry = GL_GEOMETRY_SHADER,
        Fragment = GL_FRAGMENT_SHADER
    };

    enum class ProgramProperty : uint32
    {
        UniformCount = GL_ACTIVE_UNIFORMS, 					 		/// number of uniforms present in the shader program
        MaxUniformNameLength = GL_ACTIVE_UNIFORM_MAX_LENGTH, 		/// length of the longest uniform name in the program
        AttributeCount = GL_ACTIVE_ATTRIBUTES,						/// number of attributes present in the shader program
        MaxAttributeNameLength = GL_ACTIVE_ATTRIBUTE_MAX_LENGTH,	/// length of the longest attribute name in the program
    };

    enum class IndexType : GLenum
    {
		Undefined = 0,
        u8 = GL_UNSIGNED_BYTE,
        u16 = GL_UNSIGNED_SHORT,
        u32 = GL_UNSIGNED_INT
    };

    /// Various types that OpenGL knows and that may or may not appear in shaders.
    enum class VariableType : uint32
    {
        f32         = GL_FLOAT,
        vec2        = GL_FLOAT_VEC2,
        vec3        = GL_FLOAT_VEC3,
        vec4        = GL_FLOAT_VEC4,
        int32       = GL_INT,
        ivec2       = GL_INT_VEC2,
        ivec3       = GL_INT_VEC3,
        ivec4       = GL_INT_VEC4,
        uint32      = GL_UNSIGNED_INT,
        uvec2       = GL_UNSIGNED_INT_VEC2,
        uvec3       = GL_UNSIGNED_INT_VEC3,
        uvec4       = GL_UNSIGNED_INT_VEC4,
        boolean     = GL_BOOL,
        bvec2       = GL_BOOL_VEC2,
        bvec3       = GL_BOOL_VEC3,
        bvec4       = GL_BOOL_VEC4,
        mat2        = GL_FLOAT_MAT2,
        mat3        = GL_FLOAT_MAT3,
        mat4        = GL_FLOAT_MAT4,
    //			GL_FLOAT_MAT2x3	mat2x3
    //			GL_FLOAT_MAT2x4	mat2x4
    //			GL_FLOAT_MAT3x2	mat3x2
    //			GL_FLOAT_MAT3x4	mat3x4
    //			GL_FLOAT_MAT4x2	mat4x2
    //			GL_FLOAT_MAT4x3	mat4x3
    //			GL_SAMPLER_1D	sampler1D
        sampler2D_t = GL_SAMPLER_2D,
    //			GL_SAMPLER_3D	sampler3D
    //			GL_SAMPLER_CUBE	samplerCube
    //			GL_SAMPLER_1D_SHADOW	sampler1DShadow
    //			GL_SAMPLER_2D_SHADOW	sampler2DShadow
    //			GL_SAMPLER_1D_ARRAY	sampler1DArray
    //			GL_SAMPLER_2D_ARRAY	sampler2DArray
    //			GL_SAMPLER_1D_ARRAY_SHADOW	sampler1DArrayShadow
    //			GL_SAMPLER_2D_ARRAY_SHADOW	sampler2DArrayShadow
    //			GL_SAMPLER_2D_MULTISAMPLE	sampler2DMS
    //			GL_SAMPLER_2D_MULTISAMPLE_ARRAY	sampler2DMSArray
    //			GL_SAMPLER_CUBE_SHADOW	samplerCubeShadow
    //			GL_SAMPLER_BUFFER	samplerBuffer
    //			GL_SAMPLER_2D_RECT	sampler2DRect
    //			GL_SAMPLER_2D_RECT_SHADOW	sampler2DRectShadow
    //			GL_INT_SAMPLER_1D	isampler1D
    //			GL_INT_SAMPLER_2D	isampler2D
    //			GL_INT_SAMPLER_3D	isampler3D
    //			GL_INT_SAMPLER_CUBE	isamplerCube
    //			GL_INT_SAMPLER_1D_ARRAY	isampler1DArray
    //			GL_INT_SAMPLER_2D_ARRAY	isampler2DArray
    //			GL_INT_SAMPLER_2D_MULTISAMPLE	isampler2DMS
    //			GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY	isampler2DMSArray
    //			GL_INT_SAMPLER_BUFFER	isamplerBuffer
    //			GL_INT_SAMPLER_2D_RECT	isampler2DRect
    //			GL_UNSIGNED_INT_SAMPLER_1D	usampler1D
    //			GL_UNSIGNED_INT_SAMPLER_2D	usampler2D
    //			GL_UNSIGNED_INT_SAMPLER_3D	usampler3D
    //			GL_UNSIGNED_INT_SAMPLER_CUBE	usamplerCube
    //			GL_UNSIGNED_INT_SAMPLER_1D_ARRAY	usampler2DArray
    //			GL_UNSIGNED_INT_SAMPLER_2D_ARRAY	usampler2DArray
    //			GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE	usampler2DMS
    //			GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY	usampler2DMSArray
    //			GL_UNSIGNED_INT_SAMPLER_BUFFER	usamplerBuffer
    //			GL_UNSIGNED_INT_SAMPLER_2D_RECT	usampler2DRect
        ushort_t = GL_UNSIGNED_SHORT,
    };

    struct DrawElementsIndirectCommand
    {
        GLuint count;
        GLuint instance_count;
        GLuint first_index;
        GLuint base_vertex;
        GLuint base_instance;
    };

//    enum class AccessPolicy : GLenum
//    {
//        ReadOnly = GL_READ_ONLY,
//        WriteOnly = GL_WRITE_ONLY,
//        ReadWrite = GL_READ_WRITE
//    };

//    struct SyncObject
//    {
//    public:
//        inline SyncObject(GLSync s) : _sync(s) {}

//        inline SyncObject(const SyncObject && other)
//        {
//            _sync = other._sync;
//            other._sync = 0;
//        }

//        inline SyncObject& operator=(const SyncObject && other)
//        {
//            _sync = other._sync;
//            other._sync = 0;
//        }

//        ARC_NO_COPY(SyncObject)



//    private:
//        GLsync _sync;
//    };

}} // namespace arc::gl
