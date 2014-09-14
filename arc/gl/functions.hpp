
#include "arc/core/numeric_types.hpp"
#include "arc/string/String.hpp"

#include "arc/gl/types.hpp"
#include "arc/gl/error_checking.hpp"

namespace arc { namespace gl {

inline void* map_buffer_range(BufferType target, ptrdiff_t offset, ptrdiff_t length, BufferAccess access)
{
    ARC_GL_CLEAR_ERRORS();
    auto result = glMapBufferRange((GLenum)target,offset,length,(GLbitfield)access);
    ARC_GL_CHECK_FOR_ERRORS();
    return result;
}

inline void flush_buffer_range(BufferType target, ptrdiff_t offset, ptrdiff_t length)
{
    ARC_GL_CLEAR_ERRORS();
    glFlushMappedBufferRange((GLenum)target,offset,length);
    ARC_GL_CHECK_FOR_ERRORS();
}

inline void unmap_buffer(BufferType target)
{
    ARC_GL_CLEAR_ERRORS();
    glUnmapBuffer((GLenum)target);
    ARC_GL_CHECK_FOR_ERRORS();
}

inline void gen_buffers(uint32 n, uint32* buffers)
{
    ARC_GL_CLEAR_ERRORS();
    glGenBuffers(n,buffers);
    ARC_GL_CHECK_FOR_ERRORS();
}

inline void delete_buffers(uint32 n, uint32* buffers)
{
    ARC_GL_CLEAR_ERRORS();
    glDeleteBuffers(n,buffers);
    ARC_GL_CHECK_FOR_ERRORS();
}

inline void bind_buffer(BufferType target, uint32 buffer)
{
    ARC_GL_CLEAR_ERRORS();
    glBindBuffer((GLenum)target,buffer);
    ARC_GL_CHECK_FOR_ERRORS();
}

inline void buffer_data(BufferType target, uint32 size, BufferUsage usage, void* data = nullptr)
{
    ARC_GL_CLEAR_ERRORS();
    glBufferData((GLenum)target, size, data,(GLenum)usage);
    ARC_GL_CHECK_FOR_ERRORS();
}

inline void buffer_storage(BufferType target, uint32 size, const void* data, BufferStorage flags)
{
    ARC_GL_CLEAR_ERRORS();
    glBufferStorage((GLenum)target, size, data, (GLbitfield)flags);
    ARC_GL_CHECK_FOR_ERRORS();
}
///////////////////////////////////////////////////////////////
// various                                                   //
///////////////////////////////////////////////////////////////

inline void clear_color(float r, float g, float b, float a)
{
	ARC_GL_CLEAR_ERRORS();
	glClearColor(r, g, b, a);
	ARC_GL_CHECK_FOR_ERRORS();
}

inline void clear(ClearBufferBits buffers)
{
	ARC_GL_CLEAR_ERRORS();
	glClear(static_cast<GLbitfield>(buffers));
	ARC_GL_CHECK_FOR_ERRORS();
}

///////////////////////////////////////////////////////////////
// vertex array objects (VAO)                                //
///////////////////////////////////////////////////////////////

inline void gen_vertex_arrays(uint32 n, uint32* buffer)
{
    ARC_GL_CLEAR_ERRORS();
    glGenVertexArrays(n,buffer);
    ARC_GL_CHECK_FOR_ERRORS();
}

inline void delete_vertex_arrays(uint32 n, uint32* buffer)
{
    ARC_GL_CLEAR_ERRORS();
    glDeleteVertexArrays(n,buffer);
    ARC_GL_CHECK_FOR_ERRORS();
}

inline void bind_vertex_array(uint32 id)
{
    ARC_GL_CLEAR_ERRORS();
    glBindVertexArray(id);
    ARC_GL_CHECK_FOR_ERRORS();
}

inline void enable_vertex_attribute(uint32 location)
{
    ARC_GL_CLEAR_ERRORS();
    glEnableVertexAttribArray(location);
    ARC_GL_CHECK_FOR_ERRORS();
}

inline void disable_vertex_attribute(uint32 location)
{
	ARC_GL_CLEAR_ERRORS();
	glDisableVertexAttribArray(location);
	ARC_GL_CHECK_FOR_ERRORS();
}

inline void vertex_attrib_pointer(uint32 location, uint32 element_count, uint32 type,
                                  bool normalize, uint32 stride, uint32 offset)
{
    ARC_GL_CLEAR_ERRORS();
    glVertexAttribPointer(location,element_count,type,normalize,
                          stride, reinterpret_cast<void*>(offset));
    ARC_GL_CHECK_FOR_ERRORS();
}

inline void vertex_attrib_i_pointer(uint32 location, uint32 element_count, uint32 type,
                                    uint32 stride, uint32 offset)
{
    ARC_GL_CLEAR_ERRORS();
    glVertexAttribIPointer(location,element_count, type, stride, reinterpret_cast<void*>(offset));
    ARC_GL_CHECK_FOR_ERRORS();
}

///////////////////////////////////////////////////////////////
// vertex attribute format                                   //
///////////////////////////////////////////////////////////////

inline void vertex_attrib_format(uint32 attrib_index, uint32 element_count, uint32 type,
                                 bool normalize, uint32 offset)
{
    ARC_GL_CLEAR_ERRORS();
    glVertexAttribFormat(attrib_index,element_count,type,normalize,offset);
    ARC_GL_CHECK_FOR_ERRORS();
}

inline void vertex_attrib_i_format(uint32 attrib_index, uint32 element_count, uint32 type, uint32 offset)
{
    ARC_GL_CLEAR_ERRORS();
    glVertexAttribIFormat(attrib_index,element_count,type,offset);
    ARC_GL_CHECK_FOR_ERRORS();
}

inline void vertex_attrib_binding(uint32 attrib_index, uint32 binding_index)
{
    ARC_GL_CLEAR_ERRORS();
    glVertexAttribBinding(attrib_index, binding_index);
    ARC_GL_CHECK_FOR_ERRORS();
}

inline void bind_vertex_buffer(uint32 binding_index, uint32 buffer_id, uint32 offset, uint32 stride)
{
    ARC_GL_CLEAR_ERRORS();
    glBindVertexBuffer(binding_index, buffer_id, offset, stride);
    ARC_GL_CHECK_FOR_ERRORS();
}

inline void vertex_attrib_divisor(uint32 attrib_index, uint32 divisor)
{
    ARC_GL_CLEAR_ERRORS();
    glVertexAttribDivisor(attrib_index, divisor);
    ARC_GL_CHECK_FOR_ERRORS();
}

inline void vertex_binding_divisor(uint32 binding_index, uint32 divisor)
{
    ARC_GL_CLEAR_ERRORS();
    glVertexBindingDivisor(binding_index, divisor);
    ARC_GL_CHECK_FOR_ERRORS();
}

///////////////////////////////////////////////////////////////
// shader buffer binding                                     //
///////////////////////////////////////////////////////////////

inline void bind_buffer_range(ShaderBufferTarget target, uint32 binding_index, uint32 buffer, uint32 offset, uint32 size)
{
    ARC_GL_CLEAR_ERRORS();
    glBindBufferRange((GLenum)target, binding_index, buffer, offset, size);
    ARC_GL_CHECK_FOR_ERRORS();
}

///////////////////////////////////////////////////////////////
// shaders                                                   //
///////////////////////////////////////////////////////////////

inline void use_program(uint32 id)
{
    ARC_GL_CLEAR_ERRORS();
    glUseProgram(id);
    ARC_GL_CHECK_FOR_ERRORS();
}

inline uint32 create_shader(ShaderType type)
{
    ARC_GL_CLEAR_ERRORS();
	auto id = glCreateShader((uint32)type);
    ARC_GL_CHECK_FOR_ERRORS();
    return id;
}

inline void delete_shader(uint32 id)
{
    ARC_GL_CLEAR_ERRORS();
    glDeleteShader(id);
    ARC_GL_CHECK_FOR_ERRORS();
}

inline void shader_source(uint32 shader_id, uint32 count, const char** strings, const int32* lengths)
{
    ARC_GL_CLEAR_ERRORS();
    glShaderSource(shader_id,count,strings,lengths);
    ARC_GL_CHECK_FOR_ERRORS();
}

inline bool compile_shader(uint32 shader_id)
{
    ARC_GL_CLEAR_ERRORS();
    glCompileShader(shader_id);
    ARC_GL_CHECK_FOR_ERRORS();
    GLint status;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);
    ARC_GL_CHECK_FOR_ERRORS();
    return status != GL_FALSE;
}

inline String shader_info_log(uint32 shader_id)
{
    ARC_GL_CLEAR_ERRORS();
    int len = 42;
    glGetShaderiv(shader_id,GL_INFO_LOG_LENGTH,&len);
    ARC_GL_CHECK_FOR_ERRORS();

    auto s = String::_Make_Raw(len-1);
    glGetShaderInfoLog(shader_id,len, &len,(char*)s.c_str());
    ARC_GL_CHECK_FOR_ERRORS();
    return s;
}

inline uint32 create_program()
{
    ARC_GL_CLEAR_ERRORS();
    uint32 id = glCreateProgram();
    ARC_GL_CHECK_FOR_ERRORS();
    return id;
}

inline void attach_shader(uint32 program_id, uint32 shader_id)
{
    ARC_GL_CLEAR_ERRORS();
    glAttachShader(program_id,shader_id);
    ARC_GL_CHECK_FOR_ERRORS();
}

inline bool link_program(uint32 program_id)
{
    ARC_GL_CLEAR_ERRORS();
    glLinkProgram(program_id);
    ARC_GL_CHECK_FOR_ERRORS();
    GLint status;
    glGetProgramiv(program_id, GL_LINK_STATUS, &status);
    ARC_GL_CHECK_FOR_ERRORS();
    return status != GL_FALSE;
}

inline String program_info_log(uint32 program_id)
{
    ARC_GL_CLEAR_ERRORS();
    int len = 42;
    glGetProgramiv(program_id,GL_INFO_LOG_LENGTH,&len);
    ARC_GL_CHECK_FOR_ERRORS();

    auto s = String::_Make_Raw(len-1);
    glGetProgramInfoLog(program_id,len, &len,(char*)s.c_str());
    ARC_GL_CHECK_FOR_ERRORS();
    return s;
}

inline void detach_shader(uint32 program_id, uint32 shader_id)
{
    ARC_GL_CLEAR_ERRORS();
    glDetachShader(program_id,shader_id);
    ARC_GL_CHECK_FOR_ERRORS();
}

inline void delete_program(uint32 program_id)
{
    ARC_GL_CLEAR_ERRORS();
    glDeleteProgram(program_id);
    ARC_GL_CHECK_FOR_ERRORS();
}

/// Access to different properties of a shader program.
inline uint32 program_property(uint32 program_id, ProgramProperty p)
{
    ARC_GL_CLEAR_ERRORS();
    int v;
    glGetProgramiv(program_id,(uint32)p, &v);
    ARC_GL_CHECK_FOR_ERRORS();
    return v;
}

/** Returns information about a shader attribute.
 *
 *  Params:
 * 		program_id =    ID of the shader program.
 *  	attribute_idx = Index of the attribute that should be checked.
 *  	o_size =		Returns the number of entries in case the uniform is an array variable (e.g. vec3[]).
 * 		o_type = 		Returns the tye of the shader uniform.
 * 		o_name = 		Returns the name of the shader uniform.
 */
inline void attribute_info(uint32 program_id, uint32 attribute_idx,
                           uint32& o_size, VariableType& o_type, String& o_name)
{
    uint32 max_len = gl::program_property(program_id, ProgramProperty::MaxAttributeNameLength);
    char* name_buffer = new char[max_len];

    int len;
    uint32 type;
    int32 size;

    ARC_GL_CLEAR_ERRORS();
    glGetActiveAttrib(program_id, attribute_idx, max_len, &len, &size, &type, name_buffer);
    ARC_GL_CHECK_FOR_ERRORS();

    o_name = String(name_buffer,len);
    delete[] name_buffer;
    o_type = (gl::VariableType)type;
    o_size = size;
}

inline void bind_attribute_location(uint32 program_id, uint32 attr_loc, const char* name)
{
    ARC_GL_CLEAR_ERRORS();
    glBindAttribLocation(program_id, attr_loc, name);
    ARC_GL_CHECK_FOR_ERRORS();
}

///////////////////////////////////////////////////////////////
// DrawElements                                              //
///////////////////////////////////////////////////////////////

/*
 * count: # of index entries to be used
 */
inline void draw_elements(Primitive primitive, uint32 count, IndexType index_type, uint32 index_byte_offset)
{
    ARC_GL_CLEAR_ERRORS();
	glDrawElements((GLenum)primitive, count, (GLenum)index_type, (void*)index_byte_offset);
    ARC_GL_CHECK_FOR_ERRORS();
}

inline void draw_elements_instanced_bv_bi(Primitive primitive, uint32 count, IndexType index_type,
	uint32 index_byte_offset, uint32 instance_count, uint32 vertex_count_offset, uint32 instance_offset)
{
	ARC_GL_CLEAR_ERRORS();
	glDrawElementsInstancedBaseVertexBaseInstance(
		(GLenum)primitive,
		count,
		(GLenum)index_type,
		(void*)index_byte_offset,
		instance_count,
		vertex_count_offset,
		instance_offset);
	ARC_GL_CHECK_FOR_ERRORS();
}

inline void draw_elements_indirect(Primitive primitive, IndexType index_type, uint32 byte_offset)
{
    ARC_GL_CLEAR_ERRORS();
    glDrawElementsIndirect((GLenum)primitive, (GLenum)index_type, (void*)byte_offset);
    ARC_GL_CHECK_FOR_ERRORS();
}

inline void multi_draw_elements_indirect(Primitive primitive, IndexType index_type, uint32 byte_offset,
                                         uint32 draw_count, uint32 byte_stride)
{
    ARC_GL_CLEAR_ERRORS();
    glMultiDrawElementsIndirectAMD((GLenum)primitive, (GLenum)index_type, (void*)byte_offset,
                                draw_count, byte_stride);
    ARC_GL_CHECK_FOR_ERRORS();
}

///////////////////////////////////////////////////////////////
// sync                                                      //
///////////////////////////////////////////////////////////////

inline GLsync create_fence_sync()
{
    ARC_GL_CLEAR_ERRORS();
    auto r = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE,0);
    ARC_GL_CHECK_FOR_ERRORS();
    return r;
}

inline void delete_fence_sync(GLsync sync)
{
    ARC_GL_CLEAR_ERRORS();
    glDeleteSync(sync);
    ARC_GL_CHECK_FOR_ERRORS();
}

inline WaitSyncResult client_wait_sync(GLsync sync, WaitSyncFlags flags, uint64 timeout_ns)
{
    ARC_GL_CLEAR_ERRORS();
    auto r = glClientWaitSync(sync,(GLbitfield)flags,timeout_ns);
    ARC_GL_CHECK_FOR_ERRORS();
    return (WaitSyncResult) r;
}


}} // namespace arc::gl
