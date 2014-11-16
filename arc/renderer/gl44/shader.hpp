#pragma once

#include "arc/common.hpp"
#include "arc/hash/StringHash.hpp"
#include "arc/collections/Array.hpp"
#include "arc/util/IndexPool.hpp"
#include "arc/lua/State.hpp"

#include "../types.hpp"


namespace arc { namespace renderer { 
	
	struct Config;
	
namespace gl44 {

	/* a couple of settings */
	static const uint32 MAX_UNIFORM_BLOCKS = 16;
	static const uint32 MAX_UNIFORMS = 32;
	static const uint32 MAX_VERTEX_ATTRIBUTES = 16;
	static const uint32 MAX_INSTANCE_UNIFORMS = 32;

	/* Description of a single shader. */
	struct ShaderDescription
	{
		struct Uniform
		{
			StringHash32 name;
			ShaderPrimitiveType type;
			uint8  block;
			uint16 block_offset;
		};

		struct InstanceUniformBlock
		{
			uint32 stride;				// size of the instance uniform block
			uint32 binding;				// binding index within the glsl shader
			uint32 draw_data_offset;	// offset within the draw data
		};

		struct VertexAttribute
		{
			StringHash32 name;
			bool  is_float_type;
			uint8 elements;
			uint8 location;
		};

		VertexAttribute vertex_attributes[MAX_VERTEX_ATTRIBUTES];
		uint32 vertex_attributes_count = 0;

		InstanceUniformBlock instance_uniform_blocks[MAX_UNIFORM_BLOCKS];
		uint32 instance_uniform_block_count = 0;

		Uniform instance_uniforms[MAX_INSTANCE_UNIFORMS];
		uint32 instance_uniforms_count = 0;

		uint32 gl_program_id;
		uint32 instance_uniform_draw_data_size;

		uint32 maximum_batch_size = 1;
	};

	/* Helper class managing the Instance Uniform state while submitting draw calls. */
	struct InstanceUniformSubmission
	{
	public:
		void batch_begin(uint32 batch_size);
		void batch_pre_flush(uint32 batch_size);
		void batch_post_flush(uint32 batch_size);
		void set_shader(const ShaderDescription& shader);
		void batch_enqueue(uint32 index, void* data);
	public:
		bool initialize(const Config& config);
		bool finalize();
	private:
		struct BlockState
		{
			void* buffer_data = nullptr;
			uint32 gl_id;
			uint32 buffer_offset;
			uint32 stride;
			uint32 binding;
			uint32 draw_data_offset;
		};

		struct BufferState
		{
			uint32 gl_id;
			uint32 unflushed_begin;
			uint32 mapped_end;
			uint32 front;
			void*  data = nullptr;
			bool   mapped = false;
		};

		struct UsedBuffer
		{
			uint32 gl_id;
			uint32 offset;
			uint32 length;
		};

	private:
		tuple<void*, uint32, uint32> request_buffer_memory(uint32 byte_size);	// request mapped gpu memory for the given byte_size
		void  request_new_buffer(uint32 byte_size);								// set m_current_buffer to a fresh buffer with at least byte_size space

	private:
		BlockState m_blocks[MAX_UNIFORM_BLOCKS];
		uint32     m_block_count = 0;
		uint32	   m_max_buffer_size = 1024 * 1024;		// 1MB;
		uint32     m_map_buffer_alignment;
		Array<uint32> m_available_buffers;				// OpenGL ids of unused buffers
		BufferState m_current_buffer;					// buffer memory requests are taken from this buffer until it is filled
		Array<UsedBuffer> m_used_buffers;				// buffers that need to be flushed before rendering and orphaned after rendering
	};

	/* Main class managing everything shader related. */
	class ShaderBackend
	{
	public:
		bool initialize(const Config& config);
		bool finalize();
	public:
		ShaderID create_shader(StringView lua_file_path);
	public:
		int32 get_uniform_offset(ShaderID shader_id, ShaderUniformType uniform_type, ShaderPrimitiveType type, StringHash32 name);
	public:
		Array<ShaderDescription> m_shader_data;
		IndexPool32 m_shader_indices;
	public:
		InstanceUniformSubmission m_iu_submission;
	public:
		uint32 m_max_uniform_buffer_size = 16 * 1024; // TODO: measure this at runtime (GL_MAX_UNIFORM_BLOCK_SIZE)
	public:
		lua::State m_lua;
		lua::Value m_fun_load_file;
		lua::Value m_fun_gen_code;
	};

}}}
