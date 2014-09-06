#include "mesh.hpp"

#include "arc/collections/Array.inl"
#include "arc/collections/Queue.inl"

#include "arc/gl/functions.hpp"

#include "../engine.hpp"

// Types and Variables //////////////////////////////////////////////////////////////////////////////////////////

namespace arc { namespace renderer {

	struct GeometryBufferData
	{
		uint32 size;
		InternedString name;
		uint32 gl_id;
		uint32 begin;
	};

	struct MeshData
	{
		uint32			vertex_begin;
		uint32			vertex_count;
		uint32			vertex_size;
		VertexLayout*	vertex_layout;

		uint32		index_begin;
		uint32		index_count;
		uint32		index_size;
		IndexType	index_type;

		uint32 block_begin;
		uint32 block_size;

		GeometryBufferID buffer_id;
		uint32 gl_id;
	};

	static uint32 g_gb_gl_ids[256];
	static IndexPool32 g_gb_indices;
	static Array<GeometryBufferData> g_gb_data;

	static IndexPool32 g_mesh_indices;
	static Array<MeshData> g_mesh_data;

	static uint32 g_vao_gl_ids[256];
	static IndexPool32 g_vertex_layout_indices;
	static Array<VertexLayout*> g_vertex_layout_refs;

}}

// Function Definitions /////////////////////////////////////////////////////////////////////////////////////////

namespace arc { namespace renderer {

	uint32 byte_size(IndexType t) { return (uint32)t; }

	GeometryBufferID create_geometry_buffer(InternedString name, uint32 size)
	{
		auto idx = g_gb_indices.create();

		// OpenGL
		uint32 gl_id = g_gb_gl_ids[idx];
		auto target = gl::BufferType::Array;
		gl::bind_buffer(target, gl_id);
		gl::buffer_data(target, size, gl::BufferUsage::StaticDraw);

		// book keeping
		auto& data = g_gb_data[idx];
		data.begin = 0;
		data.gl_id = gl_id;
		data.name = name;
		data.size = size;

		return GeometryBufferID{ idx };
	}

	void destroy_geometry_buffer(GeometryBufferID id)
	{
		ARC_ASSERT(id.value() <= g_gb_data.size(), "Invalid GeometryBufferID");

		uint32 gl_id = g_gb_gl_ids[id.value()];

		auto target = gl::BufferType::Array;
		gl::bind_buffer(target, gl_id);
		gl::buffer_data(target, 0, gl::BufferUsage::StaticDraw);

		// book keeping
		auto& data = g_gb_data[id.value()];
		data.begin = 0;
		data.gl_id = gl_id;
		data.name = "";
		data.size = 0;

		g_gb_indices.release(id.value());
	}

	MeshID allocate_mesh(GeometryBufferID buffer_id, uint32 index_count, uint32 vertex_count, IndexType index_type, VertexLayoutID vertex_layout_id)
	{
		ARC_NOT_IMPLEMENTED;

		ARC_ASSERT(buffer_id != INVALID_GEOMETRY_BUFFER_ID, "Invalid GeometryBufferID");
		ARC_ASSERT(buffer_id.value() <= g_gb_data.size(), "GeometryBufferID index out of bounds");
		ARC_ASSERT(g_gb_data[buffer_id.value()].size != 0, "Invalid GeometryBufferID");

		auto& buffer = g_gb_data[buffer_id.value()];
		auto& vertex_layout = *g_vertex_layout_refs[vertex_layout_id.value()];

		// check wheter the buffer has enough free memory
		uint32 vertex_stride = vertex_layout.stride();
		uint32 aligned_begin = memory::util::forward_align(buffer.begin, vertex_stride);
		uint32 vertex_size = vertex_count * vertex_stride;
		uint32 index_size = index_count * byte_size(index_type);
		uint32 block_end = aligned_begin + vertex_size + index_size;
		if (block_end > buffer.size) return INVALID_MESH_ID; // not enough memory

		uint32 index = g_mesh_indices.create();
		auto& mesh = g_mesh_data[index];

		mesh.buffer_id = buffer_id;

		mesh.vertex_begin = aligned_begin;
		mesh.vertex_count = vertex_count;
		mesh.vertex_layout = &vertex_layout;
		mesh.vertex_size = vertex_size;

		mesh.index_begin = aligned_begin + vertex_size;
		mesh.index_count = index_count;
		mesh.index_type = index_type;
		mesh.index_size = index_size;

		mesh.block_begin = buffer.begin;
		mesh.block_size = block_end - buffer.begin;

		mesh.gl_id = buffer.gl_id;

		buffer.begin = block_end;

		return MeshID{ index };
	}

	void free_mesh(MeshID mesh_id)
	{
		ARC_NOT_IMPLEMENTED;
	}

	UntypedBuffer map_mesh_vertices(MeshID mesh_id)
	{
		ARC_ASSERT(mesh_id != INVALID_MESH_ID, "Invalid MeshID");

		auto& mesh = g_mesh_data[mesh_id.value()];

		auto target = gl::BufferType::Array;
		gl::bind_buffer(target, mesh.gl_id);
		void* ptr = gl::map_buffer_range(target, mesh.vertex_begin, mesh.vertex_size,
			gl::BufferAccess::Write | gl::BufferAccess::InvalidateRange);

		if (ptr == nullptr) return INVALID_UNTYPED_BUFFER;

		return { ptr, mesh.vertex_size };
	}

	void unmap_mesh_vertices(MeshID mesh_id)
	{
		ARC_ASSERT(mesh_id != INVALID_MESH_ID, "Invalid MeshID");

		auto& mesh = g_mesh_data[mesh_id.value()];

		auto target = gl::BufferType::Array;
		gl::bind_buffer(target, mesh.gl_id);
		gl::unmap_buffer(target);
	}

	VertexLayoutID register_static_vertex_layout(VertexLayout* layout)
	{
		ARC_ASSERT(layout != nullptr, "Invalid layout");
		auto idx = g_vertex_layout_indices.create();
		g_vertex_layout_refs[idx] = layout;
		return VertexLayoutID{ idx };
	}

	void unregister_vertex_layout(VertexLayoutID id)
	{
		auto idx = id.value();
		g_vertex_layout_refs[idx] = nullptr;
		g_vertex_layout_indices.release(idx);
	}
}}


// Internals ///////////////////////////////////////////////////////////////////////////////////////////////////

namespace arc { namespace renderer { namespace internal {

	bool initialize_vertex_buffers(lua::Value& config)
	{
		const uint8 LAST_ID = 16;
		uint8 GB_SIZE_INCREMENT = 8;

		// init gl ids
		gl::gen_buffers(255, g_gb_gl_ids+1);

		// init index pool
		g_gb_indices.initialize(&engine::longterm_allocator(), LAST_ID, GB_SIZE_INCREMENT, 255, [](uint32 new_size)
		{
			GeometryBufferData gbd; gbd.size = 0;
			g_gb_data.resize(new_size, gbd);
		});

		// init data store
		g_gb_data.initialize(&engine::longterm_allocator(), 0);
		GeometryBufferData gbd; gbd.size = 0;
		g_gb_data.resize(1 + LAST_ID, gbd);

		return true;
	}

	bool finalize_vertex_buffers()
	{
		//TODO check whether initialized
		gl::delete_buffers(255, g_gb_gl_ids+1);
		g_gb_indices.finalize();
		g_gb_data.finalize();

		return true;
	}

	bool initialize_vertex_layout_backend()
	{
		const uint8 LAST_ID = 16;

		gl::gen_vertex_arrays(255, g_vao_gl_ids + 1);

		g_vertex_layout_indices.initialize(&engine::longterm_allocator(), LAST_ID, 8, 255, [](uint32 new_size)
		{
			g_vertex_layout_refs.resize(new_size, nullptr);
		});

		g_vertex_layout_refs.initialize(&engine::longterm_allocator(), LAST_ID + 1);

		return true;
	}

	bool finalize_vertex_layout_backend()
	{
		gl::delete_vertex_arrays(255, g_vao_gl_ids + 1);
		return true;
	}
}}}


namespace arc { namespace renderer { 

	struct DefaultCommandData
	{
		uint32 size;
		MeshID mesh;
	};

	uint32 RenderContext::required_data_size(uint32 max_command_count, uint32 max_data_size)
	{
		uint32 align_overhead = 8 * max_command_count;
		uint32 user_data = max_data_size;
		uint32 default_data = sizeof(DefaultCommandData) * max_command_count;
		return align_overhead + user_data + default_data;
	}

	void Renderer_GL44::update_frame_begin()
	{
		m_frame_alloc.reset();
		m_frame_counter.increment();
	}

	RenderContext* Renderer_GL44::get_render_context(uint32 max_command_count, uint32 max_data_size)
	{
		//TODO make thread safe

		auto rc = m_frame_alloc.create<RenderContext>();
		if (rc == nullptr) ARC_FAIL_GRACEFULLY_MESSAGE("Could not allocate RenderContext.");

		uint32 total_data_size = rc->required_data_size(max_command_count, max_data_size);
		rc->m_data_begin = (char*)m_frame_alloc.allocate(total_data_size, 8);
		rc->m_data_current = rc->m_data_begin;
		rc->m_data_end = rc->m_data_begin + total_data_size;
		if (rc->m_data_begin == nullptr) ARC_FAIL_GRACEFULLY_MESSAGE("Could not allocate RenderContext.");
		
		rc->m_max_command_count = max_command_count;
		rc->m_count = 0;
		rc->m_commands = m_frame_alloc.create_n<RenderCommand>(max_command_count);
		if (rc->m_commands == nullptr) ARC_FAIL_GRACEFULLY_MESSAGE("Could not allocate RenderContext.");

		return rc;
	}

	UntypedBuffer RenderContext::add_command(ShaderID shader, MeshID mesh, float depth, uint32 data_size)
	{
		auto& cmd = m_commands[m_count];

		// allocate the data section
		cmd.data = memory::util::forward_align_ptr(m_data_current, 8);
		auto dcd = (DefaultCommandData*)cmd.data;
		dcd->mesh = mesh;
		dcd->size = data_size;
		m_data_current += sizeof(DefaultCommandData) + data_size;
		if (m_data_current > m_data_end) return INVALID_UNTYPED_BUFFER;

		// create the sort key
		ARC_NOT_IMPLEMENTED;

		return UntypedBuffer{ dcd + 1, data_size };
	}

}}