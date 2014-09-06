#pragma once

#include "arc/common.hpp"
#include "arc/lua/State.hpp"
#include "arc/collections/Array.hpp"
#include "arc/util/IndexPool.hpp"
#include "arc/memory/LinearAllocator.hpp"
#include "arc/util/Counter.hpp"

#include "VertexLayout.hpp"

// Type Definitions /////////////////////////////////////////////////////////////////////////////////////////////

namespace arc { namespace renderer {

	struct _MeshIDTag {};
	struct _GeometryIDHandleTag {};
	struct _ShaderIDTag {};
	struct _RenderPassIDTag {};
	struct _VertexLayoutIDTag {};

	using MeshID = Index32<_MeshIDTag>;
	using GeometryBufferID = Index32<_GeometryIDHandleTag>;
	using ShaderID = Index32 < _ShaderIDTag > ;
	using RenderPassID = Index32 < _RenderPassIDTag > ;
	using VertexLayoutID = Index32 < _VertexLayoutIDTag > ;

	using InternedString = const char*;

	static const MeshID INVALID_MESH_ID = MeshID(-1);
	static const GeometryBufferID INVALID_GEOMETRY_BUFFER_ID = GeometryBufferID(0);

	struct UntypedBuffer
	{
		void*  ptr;
		size_t size;

		inline bool valid() { return ptr != nullptr; }
	};

	static const UntypedBuffer INVALID_UNTYPED_BUFFER = { nullptr, 0 };

}}

// Function Definitions /////////////////////////////////////////////////////////////////////////////////////////

namespace arc { namespace renderer {

	VertexLayoutID register_static_vertex_layout(VertexLayout* layout);
	void unregister_vertex_layout(VertexLayoutID id);

	GeometryBufferID create_geometry_buffer(InternedString name, uint32 size);
	void destroy_geometry_buffer(GeometryBufferID h);

	MeshID allocate_mesh(GeometryBufferID buffer, uint32 index_count, uint32 vertex_count, IndexType index_type, VertexLayoutID vertex_layout_id);
	void free_mesh(MeshID mesh_id);

	UntypedBuffer map_mesh_vertices(MeshID mesh_id);
	void unmap_mesh_vertices(MeshID mesh_id);

}}

namespace arc { namespace renderer {

	class Renderer_GL44;

	union SortKey
	{
		struct
		{
			uint64 target_config : 16;
			uint64 depth : 16;
			uint64 buffer : 8;
			uint64 shader : 8;
			uint64 shader_variation : 8;
			uint64 various : 8;
		} fields;
		uint64 integer;
	};

	struct RenderCommand
	{
		SortKey sort_key;
		void* data;
	};

	class RenderContext
	{
	public:
		UntypedBuffer add_command(ShaderID shader, MeshID mesh, float depth, uint32 data_size);
	private:
		uint32 required_data_size(uint32 max_command_count, uint32 max_data_size);
	private:
		char* m_data_begin;
		char* m_data_end;
		char* m_data_current;

		RenderCommand* m_commands;
		uint32 m_max_command_count;
		uint32 m_count;
	private:
		friend class Renderer_GL44;
	};

	enum class PrimitiveType : uint8
	{
		Triangle = 0,
	};

	class Renderer_GL44
	{
	public:
		void update_frame_begin();
	public:
		RenderContext* get_render_context(uint32 max_command_count, uint32 max_data_size);
		void submit_render_context(RenderContext* render_context);
	private:
		memory::LinearAllocator m_frame_alloc;
		Counter32 m_frame_counter;
	};

	void prepare_shader(ShaderID shader_id);
	void prepare_geometry_buffer();

	inline SortKey encode_sort_key(uint16 target_config, uint16 depth, GeometryBufferID buffer, ShaderID shader, uint8 shader_variation)
	{
		SortKey sk;
		sk.fields.target_config = target_config;
		sk.fields.depth = depth;
		sk.fields.buffer = buffer.value();
		sk.fields.shader = shader.value();
		sk.fields.shader_variation = shader_variation;
		sk.fields.various = 0;
		return sk;
	}

}}


// Internals ///////////////////////////////////////////////////////////////////////////////////////////////////

namespace arc { namespace renderer { namespace internal {

	bool initialize_vertex_buffers(lua::Value& config);
	bool finalize_vertex_buffers();

}}}