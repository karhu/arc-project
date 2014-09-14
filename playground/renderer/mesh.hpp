#pragma once
#if 0
#include "arc/common.hpp"
#include "arc/lua/State.hpp"
#include "arc/collections/Array.hpp"
#include "arc/util/IndexPool.hpp"
#include "arc/memory/LinearAllocator.hpp"
#include "arc/util/Counter.hpp"

#include "VertexLayout.hpp"
#include "types.hpp"

// Type Definitions /////////////////////////////////////////////////////////////////////////////////////////////

namespace arc { namespace renderer {

	DECLARE_ID32(BufferID);
	static const BufferID        INVALID_BUFFER_ID		  = BufferID(0);

	using InternedString = const char*;

}}

namespace arc { namespace renderer {

	struct InstanceUniformInfo
	{
		StringHash32 name_hash;
		uint8 size;

		// opengl
		uint8 binding;
		uint8 buffer;
		uint8 offset;

		// render context
		uint16 draw_data_offset;
	};

	struct InstanceUniformBlock
	{
		void*  ptr;
		uint32 gl_buffer_id;

		uint16 draw_data_offset;
		uint16 stride;
	};

	struct ShaderData
	{
		uint32 gl_id;
		InternedString name;

		uint16 draw_data_size;
		uint16 draw_data_offset;

		InstanceUniformInfo* iu_data;
		uint16 iu_count;

		InstanceUniformBlock* iu_block_data;
		uint8  iu_block_count;

		uint16 max_batch_size;

		uint16 batch_begin_idx;
		uint16 batch_end_idx;
		uint16 batch_current_idx;
	};
}}

// Function Definitions /////////////////////////////////////////////////////////////////////////////////////////

namespace arc { namespace renderer {

	class Renderer_GL44;

	struct RenderCommand
	{
		uint64 sort_key;
		void* data;
	};

	struct BufferData
	{
		uint32 size;
		uint32 gl_id;
		uint32 front;
		InternedString name;
	};

	struct VertexLayoutData
	{
		VertexLayout* ref = nullptr;
		uint32 gl_vao;
	};

	struct MeshData
	{
		uint32			vertex_begin;
		uint32			vertex_count;
		uint32			vertex_size;
		VertexLayoutID	vertex_layout;

		uint32		index_begin;
		uint32		index_count;
		uint32		index_size;
		IndexType	index_type;

		uint32 block_begin;
		uint32 block_size;

		BufferID buffer_id;
		uint32 gl_id;
	};

	class RenderContext
	{
	public:
		void* add_command(ShaderID shader, GeometryID mesh, uint16 depth, uint32 data_size);
	private:
		uint32 required_data_size(uint32 max_command_count, uint32 max_data_size);
	private:
		char* m_data_begin;
		char* m_data_end;
		char* m_data_current;

		RenderCommand* m_commands;
		uint32 m_max_command_count;
		uint32 m_count;
		Renderer_GL44* m_renderer;
	private:
		friend class Renderer_GL44;
	};

	class Renderer_GL44
	{
	public:
		Renderer_GL44() = default;
	public:
		bool initialize();
		void finalize();
		bool is_initialized();
	public:
		void update_frame_begin();
	public:
		void render_frame();
	public:
		BufferID create_buffer(InternedString name, uint32 size);
		void destroy_buffer(BufferID id);
	public:
		GeometryID allocate_geometry(BufferID buffer, uint32 index_count, uint32 vertex_count, IndexType index_type, VertexLayoutID vertex_layout);
		void free_geometry(GeometryID geometry);
		UntypedBuffer map_geometry_vertices(GeometryID geometry_id);
		void unmap_geometry_vertices(GeometryID geometry_id);
		BufferID get_buffer(GeometryID geometry_id);
		VertexLayoutID get_layout(GeometryID geometry_id);
	public:
		VertexLayoutID register_static_vertex_layout(VertexLayout* layout);
		void unregister_vertex_layout(VertexLayoutID id);
	public:
		RenderContext* create_render_context(uint32 max_command_count, uint32 max_data_size);
		void submit_render_context(RenderContext* render_context);
	public:
		uint32 shader_get_instance_uniform_offset(ShaderID shader, StringHash32 name);
	private:
		void enable_render_pass(uint16 id);
		void enable_buffer(uint8 buffer, uint8 vertex_layout);
		void enable_shader(uint8 shader, uint8 shader_variation);
		void enable_shader_variation(uint8 shader_variation);
		void enable_vertex_layout(uint8 buffer, uint8 vertex_layout);
	private:
		memory::LinearAllocator m_frame_alloc;
		Counter32 m_frame_counter;
	private:
		uint32 m_buffer_gl_ids[256];
		IndexPool32 m_buffer_indices;
		Array<BufferData> m_buffer_data;
	private:
		uint32 m_vao_gl_ids[256];
		IndexPool32 m_vertex_layout_indices;
		Array<VertexLayoutData> m_vertex_layout_data;
	private:
		IndexPool32 m_geometry_indices;
		Array<MeshData> m_geometry_data;
	private:
		Array<ShaderData> m_shader_data;
	private:
		Array<RenderContext*> m_submitted_render_contexts;
	};

}}

// Internals ///////////////////////////////////////////////////////////////////////////////////////////////////

namespace arc { namespace renderer { namespace internal {

	bool initialize_vertex_buffers(lua::Value& config);
	bool finalize_vertex_buffers();

}}}

#endif