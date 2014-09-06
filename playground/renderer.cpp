#if 0

#include "renderer.hpp"

#include "arc/collections/Array.inl"
#include "arc/gl/functions.hpp"

#include "engine.hpp"

namespace arc { namespace renderer {

	// types /////////////////////////////////////////////////////////////////////////////////////////

	struct BufferFreeBlock
	{
		uint32 begin;
		uint32 size;
	};

	struct MeshMetaData
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

		uint32 gl_id;
	};

	struct MeshBufferData
	{
		uint32 gl_id = 0;
		uint32 size = 0;

		Array<MeshMetaData> meshes;
		Array<uint8> generations;

		Array<BufferFreeBlock> free_blocks;
	};

	// globals //////////////////////////////////////////////////////////////////////////////////////////

	bool   g_is_initialized = false;
	MeshBufferData g_static_mesh_buffer;
	
	// internal functions ///////////////////////////////////////////////////////////////////////////////

	uint32 byte_size(IndexType t) { return (uint32)t; }

	// functions ////////////////////////////////////////////////////////////////////////////////////////

	bool initialize(lua::Value& config) 
	{
		if (g_is_initialized) return true;

		// TODO: read lua config
		uint32 static_mesh_buffer_size = 64 * 1024 * 1024; // 64MB
		uint32 static_mesh_init_count = 128;
		uint32 static_mesh_grow_count = 128;

		// allocate static mesh facilities
		auto& buffer = g_static_mesh_buffer;
		buffer.size = static_mesh_buffer_size;
		buffer.free_blocks.push_back({ 0, buffer.size });

		buffer.generations.initialize(&engine::longterm_allocator(), static_mesh_init_count);
		buffer.meshes.initialize(&engine::longterm_allocator(), static_mesh_init_count);
		buffer.free_blocks.initialize(&engine::longterm_allocator(), 16);

		auto target = gl::BufferType::Array;
		gl::gen_buffers(1, &buffer.gl_id);
		gl::bind_buffer(target, buffer.gl_id);
		gl::buffer_data(target, buffer.size, gl::BufferUsage::StaticDraw);

		return true;
	}

	static const MeshHandle INVALID_MESH_HANDLE = { 0, 0 };

	MeshHandle allocate_static_mesh(uint32 index_count, uint32 vertex_count, IndexType index_type, VertexLayout* vertex_layout)
	{
		ARC_ASSERT(is_initialized(), "renderer system is not initialized");
		ARC_ASSERT(vertex_layout != nullptr, "vertex_layout is null");

		auto& buffer = g_static_mesh_buffer;

		// calculate required size
		uint32 index_size = index_count * byte_size(index_type);
		uint32 vertex_stride = vertex_layout->stride();
		uint32 vertex_size = vertex_count * vertex_stride;
		uint32 total_size = index_size + vertex_size;

		// find free and large enough buffer block
		BufferFreeBlock* bfb = nullptr;
		uint32 aligned_begin;
		for (auto& fb : buffer.free_blocks)
		{

			aligned_begin = memory::util::forward_align(fb.begin, vertex_stride);
			uint32 available = fb.size - (aligned_begin - fb.begin);
			if (available >= total_size)
			{
				bfb = &fb;
				break;
			}
		}
		// we didn't find a good block
		if (bfb == nullptr) return INVALID_MESH_HANDLE;

		// setup mesh data
		MeshMetaData mesh;

		mesh.block_begin = bfb->begin;
		mesh.block_size = (aligned_begin - bfb->begin) + total_size;

		mesh.gl_id = buffer.gl_id;

		mesh.vertex_begin = aligned_begin;
		mesh.vertex_count = vertex_count;
		mesh.vertex_layout = vertex_layout;
		mesh.vertex_size = vertex_size;

		mesh.index_begin = mesh.vertex_begin + mesh.vertex_size;
		mesh.index_count = index_count;
		mesh.index_type = index_type;
		mesh.index_size = index_size;

		// update free block

		//TODO: bag_remove(Array<T> array, uint32 index);

		ARC_NOT_IMPLEMENTED;
	}

	bool finalize() { ARC_NOT_IMPLEMENTED; }

	bool is_initialized() { return g_is_initialized; }

	void invalidate_vertex_layout(VertexLayout* ptr_layout) { ARC_NOT_IMPLEMENTED;  }

}} // namespace arc::renderer

#endif