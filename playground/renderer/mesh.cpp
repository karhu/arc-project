#include "mesh.hpp"

#if 0

#include "arc/collections/Array.inl"
#include "arc/collections/Queue.inl"
#include "arc/math/common.hpp"

#include "arc/gl/functions.hpp"

#include "../engine.hpp"

//TODO remove
#include <iostream>
#include "arc/collections/Array.inl"

// Types and Variables //////////////////////////////////////////////////////////////////////////////////////////

namespace arc { namespace renderer {

	union SortKey
	{
		struct
		{
			//TODO one bit for index_type
			uint64 vertex_layout : 8; //TODO do we need vertex_layout_id?
			uint64 shader_variation : 8;
			uint64 buffer : 8;
			uint64 shader : 8;
			uint64 depth : 16;
			uint64 target_config : 16;
		} fields;
		uint64 integer;

		static SortKey Encode(uint16 target_config, uint16 depth, BufferID buffer, ShaderID shader, uint8 shader_variation, VertexLayoutID vertex_layout);
		static SortKey Decode(uint64 value);

	};

	uint32 DEFAULT_BINDING_INDEX_GL = 0;

}}

// Shader Stuff /////////////////////////////////////////////////////////////////////////////////////////

namespace arc { namespace renderer { 

	uint16 batch_prepare(ShaderData& self, uint16 size)
	{
		// make sure we have available space in our instance uniform buffers
		if (self.batch_end_idx == self.max_batch_size)
		{
			auto target = gl::BufferType::Uniform;

			// orphan the old buffers, get new ones, map them
			for (uint32 i = 0; i < self.iu_block_count; i++)
			{
				auto& iub = self.iu_block_data[i];
				
				gl::bind_buffer(target, iub.gl_buffer_id);
				gl::buffer_data(target, iub.stride*self.max_batch_size, gl::BufferUsage::StreamDraw, nullptr);
				gl::map_buffer_range(target, 0, iub.stride*self.max_batch_size,
					gl::BufferAccess::Write |
					gl::BufferAccess::Unsynchronized |
					gl::BufferAccess::InvalidateBuffer |
					gl::BufferAccess::FlushExplicit);
			}
		}

		// book keeping
		uint16 available = self.max_batch_size - self.batch_end_idx;
		uint16 this_batch_size = min(available, size);
		self.batch_begin_idx = self.batch_end_idx;
		self.batch_end_idx = self.batch_begin_idx + this_batch_size;
		self.batch_current_idx = self.batch_begin_idx;
		
		return this_batch_size;
	}

	void batch_entry(ShaderData& self, void* data_ptr)
	{
		for (uint32 i = 0; i < self.iu_block_count; i++)
		{
			auto& iub = self.iu_block_data[i];
			
			void* gpu_memory = memory::util::ptr_add(iub.ptr, iub.stride*self.batch_current_idx);
			void* cpu_memory = memory::util::ptr_add(data_ptr, iub.draw_data_offset);
			memcpy(gpu_memory, cpu_memory, iub.stride);
		}
		self.batch_current_idx += 1;
	}

	void batch_finish(ShaderData& self)
	{
		auto target = gl::BufferType::Uniform;

		// flush all instance uniform buffers
		for (uint32 i = 0; i < self.iu_block_count; i++)
		{
			auto& iub = self.iu_block_data[i];
			uint16 current_batch_size = self.batch_current_idx - self.batch_begin_idx;
			gl::bind_buffer(target, iub.gl_buffer_id);
			gl::flush_buffer_range(target, self.batch_begin_idx*iub.stride, current_batch_size*iub.stride);
			gl::unmap_buffer(target);
		}
	}

	uint32 Renderer_GL44::shader_get_instance_uniform_offset(ShaderID shader_id, StringHash32 name)
	{
		//TODO: ARC_ASSERT(m_geometry_indices.valid(id.value()), "Invalid GeometryID");
		auto& sd = m_shader_data[shader_id.value()];

		for (uint16 i = 0; i < sd.iu_count; i++)
		{
			auto& iu = sd.iu_data[i];
			if (iu.name_hash == name) return iu.draw_data_offset;
		}
		ARC_ASSERT(false, "Instance uniform name not found.");
		return -1;
	}

}}

namespace arc { namespace renderer { 

	struct DefaultCommandData
	{
		uint32 size;
		GeometryID geometry;
	};

	uint32 RenderContext::required_data_size(uint32 max_command_count, uint32 max_data_size)
	{
		uint32 align_overhead = 8 * max_command_count;
		uint32 user_data = max_data_size;
		uint32 default_data = sizeof(DefaultCommandData) * max_command_count;
		return align_overhead + user_data + default_data;
	}

	void* RenderContext::add_command(ShaderID shader, GeometryID geometry, uint16 depth, uint32 data_size)
	{
		auto& cmd = m_commands[m_count];

		// allocate the data section
		cmd.data = memory::util::forward_align_ptr(m_data_current, 8);
		auto dcd = (DefaultCommandData*)cmd.data;

		dcd->geometry = geometry;
		dcd->size = data_size;
		m_data_current += sizeof(DefaultCommandData) + data_size;
		if (m_data_current > m_data_end) return nullptr;

		// create the sort key
		//cmd.sort_key = SortKey::Encode(0, depth, m_renderer->get_buffer(geometry), shader, 0, m_renderer->get_layout(geometry));
		cmd.sort_key = SortKey::Encode(0, 0, BufferID(geometry.value()), shader, 0, VertexLayoutID(2)).integer;

		m_count += 1;

		return dcd + 1 ;
	}

	/*static*/ SortKey SortKey::Encode(uint16 target_config, uint16 depth, BufferID buffer, ShaderID shader, uint8 shader_variation, VertexLayoutID vertex_layout)
	{
		SortKey sk;
		sk.fields.target_config = target_config;
		sk.fields.depth = depth;
		sk.fields.buffer = buffer.value();
		sk.fields.shader = shader.value();
		sk.fields.shader_variation = shader_variation;
		sk.fields.vertex_layout = vertex_layout.value();
		return sk;
	}

	/*static*/ SortKey SortKey::Decode(uint64 value)
	{
		SortKey sk;
		sk.integer = value;
		return sk;
	}
}}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Renderer_GL44 //////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace arc { namespace renderer { 

	bool Renderer_GL44::initialize()
	{
		// buffers /////////////////////////////////////////////////////////////////////

		const uint8 BUFFER_LAST_ID_BEFORE_INCREMENT = 16;
		const uint8 BUFFER_SIZE_INCREMENT = 8;

		// init gl ids
		gl::gen_buffers(255, m_buffer_gl_ids + 1);

		// init index pool
		m_buffer_indices.initialize(
			&engine::longterm_allocator(), 
			BUFFER_LAST_ID_BEFORE_INCREMENT, 
			BUFFER_SIZE_INCREMENT, 
			255, 
			[this](uint32 new_size) {
				BufferData bd; bd.size = 0;
				m_buffer_data.resize(new_size, bd);
			}
		);

		// init data store
		m_buffer_data.initialize(&engine::longterm_allocator(), 0);
		BufferData bd; bd.size = 0;
		m_buffer_data.resize(1 + BUFFER_LAST_ID_BEFORE_INCREMENT, bd);

		// vertex layout ////////////////////////////////////////////////////////////////

		const uint8 VL_LAST_ID_BEFORE_INC = 16;
		const uint8 VL_SIZE_INC = 8;

		// open gl
		gl::gen_vertex_arrays(255, m_vao_gl_ids + 1);

		// ids
		m_vertex_layout_indices.initialize(
			&engine::longterm_allocator(), 
			VL_LAST_ID_BEFORE_INC, 
			VL_SIZE_INC, 
			255, 
			[this](uint32 new_size) { m_vertex_layout_data.resize(new_size); }
		);

		// data
		m_vertex_layout_data.initialize(&engine::longterm_allocator(), VL_LAST_ID_BEFORE_INC + 1);

		// geometry /////////////////////////////////////////////////////////////////////

		static uint32 GEOM_LAST_ID_BEFORE_INC = 64;
		static uint32 GEOM_SIZE_INC = 64;
		static uint32 GEOM_MAX_ID = 20000000;

		m_geometry_indices.initialize(
			&engine::longterm_allocator(),
			GEOM_LAST_ID_BEFORE_INC,
			GEOM_SIZE_INC,
			GEOM_MAX_ID,
			[this](uint32 new_size) { m_geometry_data.resize(new_size); }
		);

		// data
		m_geometry_data.initialize(&engine::longterm_allocator(), GEOM_LAST_ID_BEFORE_INC + 1);

		// render context ///////////////////////////////////////////////////////////////

		m_frame_alloc.initialize(&engine::longterm_allocator(), 1024 * 1024 * 8); // 8MB

		m_submitted_render_contexts.initialize(&engine::longterm_allocator());
		m_submitted_render_contexts.reserve(8);

		return true;
	}

	void Renderer_GL44::finalize() 
	{
		// check whether already initialized
		if (!m_buffer_data.is_initialized()) return;

		// buffer ////////////////////////////////////////////

		gl::delete_buffers(255, m_buffer_gl_ids + 1);
		m_buffer_indices.finalize();
		m_buffer_data.finalize();

		// vertex layout /////////////////////////////////////

		gl::delete_vertex_arrays(255, m_vao_gl_ids + 1);
		m_vertex_layout_indices.finalize();
		m_vertex_layout_data.finalize();

		// geometry //////////////////////////////////////////

		m_geometry_data.finalize();
		m_geometry_indices.finalize();

		// render context ////////////////////////////////////

		m_submitted_render_contexts.finalize();

		// shaders ///////////////////////////////////////////

		//m_shader_data.finalize();
	}

	bool Renderer_GL44::is_initialized()
	{
		ARC_NOT_IMPLEMENTED;
	}

	void Renderer_GL44::update_frame_begin()
	{
		m_frame_alloc.reset();
		m_frame_counter.increment();
	}

	BufferID Renderer_GL44::create_buffer(InternedString name, uint32 size)
	{
		auto idx = m_buffer_indices.create();

		// OpenGL
		uint32 gl_id = m_buffer_gl_ids[idx];
		auto target = gl::BufferType::Array;
		gl::bind_buffer(target, gl_id);
		gl::buffer_data(target, size, gl::BufferUsage::StaticDraw);

		// book keeping
		auto& data = m_buffer_data[idx];
		data.front = 0;
		data.gl_id = gl_id;
		data.name = name;
		data.size = size;

		return BufferID{ idx };
	}

	void Renderer_GL44::destroy_buffer(BufferID id)
	{
		ARC_ASSERT(id.value() <= m_buffer_data.size(), "Invalid BufferID");

		uint32 gl_id = m_buffer_gl_ids[id.value()];

		auto target = gl::BufferType::Array;
		gl::bind_buffer(target, gl_id);
		gl::buffer_data(target, 0, gl::BufferUsage::StaticDraw);

		// book keeping
		auto& data = m_buffer_data[id.value()];
		data.front = 0;
		data.gl_id = gl_id;
		data.name = "";
		data.size = 0;

		m_buffer_indices.release(id.value());
	}

	GeometryID Renderer_GL44::allocate_geometry(BufferID buffer_id, uint32 index_count, uint32 vertex_count, IndexType index_type, VertexLayoutID vertex_layout_id)
	{
		ARC_ASSERT(buffer_id != INVALID_BUFFER_ID, "Invalid BufferID");
		ARC_ASSERT(buffer_id.value() <= m_buffer_data.size(), "BufferID index out of bounds");
		ARC_ASSERT(m_buffer_data[buffer_id.value()].size != 0, "Invalid BufferID");

		auto& buffer = m_buffer_data[buffer_id.value()];
		auto& vertex_layout = *m_vertex_layout_data[vertex_layout_id.value()].ref;

		// check wheter the buffer has enough free memory
		uint32 vertex_stride = vertex_layout.stride();
		uint32 aligned_begin = memory::util::forward_align(buffer.front, vertex_stride);
		uint32 vertex_size = vertex_count * vertex_stride;
		uint32 index_size = index_count * byte_size(index_type);
		uint32 block_end = aligned_begin + vertex_size + index_size;
		if (block_end > buffer.size) return INVALID_GEOMETRY_ID; // not enough memory

		uint32 index = m_geometry_indices.create();
		auto& mesh = m_geometry_data[index];

		mesh.buffer_id = buffer_id;

		mesh.vertex_begin = aligned_begin;
		mesh.vertex_count = vertex_count;
		mesh.vertex_size = vertex_size;

		mesh.index_begin = aligned_begin + vertex_size;
		mesh.index_count = index_count;
		mesh.index_type = index_type;
		mesh.index_size = index_size;

		mesh.block_begin = buffer.front;
		mesh.block_size = block_end - buffer.front;

		mesh.gl_id = buffer.gl_id;
		mesh.vertex_layout = vertex_layout_id;

		buffer.front = block_end;

		return GeometryID{ index };
	}

	void Renderer_GL44::free_geometry(GeometryID geometry)
	{
		ARC_NOT_IMPLEMENTED
	}

	VertexLayoutID Renderer_GL44::register_static_vertex_layout(VertexLayout* layout)
	{
		ARC_ASSERT(layout != nullptr, "Invalid layout");
		auto idx = m_vertex_layout_indices.create();
		m_vertex_layout_data[idx].ref = layout;

		// TODO create vao

		return VertexLayoutID{ idx };
	}

	void Renderer_GL44::unregister_vertex_layout(VertexLayoutID id)
	{
		auto idx = id.value();
		m_vertex_layout_data[idx].ref = nullptr;
		m_vertex_layout_indices.release(idx);
	}

	RenderContext* Renderer_GL44::create_render_context(uint32 max_command_count, uint32 max_data_size)
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

		rc->m_renderer = this;

		return rc;
	}

	UntypedBuffer Renderer_GL44::map_geometry_vertices(GeometryID geometry_id)
	{
		ARC_ASSERT(m_geometry_indices.valid(geometry_id.value()),"Invalid GeometryID");

		auto& mesh = m_geometry_data[geometry_id.value()];

		auto target = gl::BufferType::Array;
		gl::bind_buffer(target, mesh.gl_id);
		void* ptr = gl::map_buffer_range(target, mesh.vertex_begin, mesh.vertex_size,
			gl::BufferAccess::Write | gl::BufferAccess::InvalidateRange);

		if (ptr == nullptr) return INVALID_UNTYPED_BUFFER;

		return{ ptr, mesh.vertex_size };
	}

	void Renderer_GL44::unmap_geometry_vertices(GeometryID geometry_id)
	{
		ARC_ASSERT(m_geometry_indices.valid(geometry_id.value()), "Invalid GeometryID");

		auto& mesh = m_geometry_data[geometry_id.value()];

		auto target = gl::BufferType::Array;
		gl::bind_buffer(target, mesh.gl_id);
		gl::unmap_buffer(target);
	}

	BufferID Renderer_GL44::get_buffer(GeometryID id)
	{
		ARC_ASSERT(m_geometry_indices.valid(id.value()), "Invalid GeometryID");
		return m_geometry_data[id.value()].buffer_id;
	}

	void Renderer_GL44::submit_render_context(RenderContext* context)
	{
		// TODO: make thread safe

		// sort commands in this context
		std::sort(context->m_commands, context->m_commands + context->m_count, [](const RenderCommand& a, const RenderCommand& b)
		{
			return a.sort_key < b.sort_key;
		});

		m_submitted_render_contexts.push_back(context);
	}

	VertexLayoutID Renderer_GL44::get_layout(GeometryID id)
	{
		ARC_ASSERT(m_geometry_indices.valid(id.value()), "Invalid GeometryID");
		return m_geometry_data[id.value()].vertex_layout;
	}

	void Renderer_GL44::render_frame()
	{
		//TODO merge sort
		RenderCommand* m_render_command_data = m_submitted_render_contexts.back()->m_commands;
		uint32 m_render_command_count = m_submitted_render_contexts.back()->m_count;

		if (m_render_command_count == 0) return;

		auto batch_key = SortKey::Decode(m_render_command_data[0].sort_key);
		Array<uint64> draws(engine::longterm_allocator(), 0);

		// change states
		enable_render_pass(batch_key.fields.target_config);
		enable_buffer(batch_key.fields.buffer, batch_key.fields.vertex_layout);
		enable_shader(batch_key.fields.shader, batch_key.fields.shader_variation);
		enable_shader_variation(batch_key.fields.shader_variation);

		for (uint32 i = 0; i < m_render_command_count; i++)
		{
			auto& cmd = m_render_command_data[i];
			auto  key = SortKey::Decode(cmd.sort_key);

			// mask out depth
			auto key_dm = key; key_dm.fields.depth = 0;
			auto batch_key_dm = batch_key; batch_key_dm.fields.depth = 0;
			
			// render commands differ in more than depth
			if (key_dm.integer != batch_key_dm.integer)
			{
				// render enqueued
				for (auto m : draws) { std::cout << "   " << m << std::endl; }
				draws.clear();
				
				// change states
				if (key.fields.target_config != batch_key.fields.target_config) enable_render_pass(key.fields.target_config);
				if (key.fields.buffer != batch_key.fields.buffer)
				{
					enable_buffer(key.fields.buffer, key.fields.vertex_layout);
					// prevent repeated vertex layout change
					batch_key.fields.vertex_layout = key.fields.vertex_layout;
				}
				if (key.fields.shader != batch_key.fields.shader) enable_shader(key.fields.shader, key.fields.shader_variation);
				if (key.fields.shader_variation != batch_key.fields.shader_variation) enable_shader_variation(key.fields.shader_variation);
				if (key.fields.vertex_layout != batch_key.fields.vertex_layout) enable_vertex_layout(key.fields.buffer, key.fields.vertex_layout);
				
				batch_key = key;
			}

			// enqueue current cmd
			void* data = (char*)cmd.data + sizeof(DefaultCommandData);
			draws.push_back(*((uint64*)data));
		}

		// render enqueued
		for (auto m : draws) { std::cout << "   " << m << std::endl; }
		draws.clear();
	}

	void Renderer_GL44::enable_render_pass(uint16 id)
	{
		std::cout << "[RenderPass] " << id << std::endl;
	}

	void Renderer_GL44::enable_buffer(uint8 buffer, uint8 vertex_layout)
	{
		std::cout << "[Buffer] " << (uint32)buffer << std::endl;

		auto&  layout    = *m_vertex_layout_data[vertex_layout].ref;
		uint32 gl_vao    = m_vertex_layout_data[vertex_layout].gl_vao;
		uint32 gl_buffer = m_buffer_gl_ids[buffer];

		gl::bind_vertex_array(gl_vao);
		gl::bind_buffer(gl::BufferType::ElementArray, gl_buffer);
		gl::bind_vertex_buffer(DEFAULT_BINDING_INDEX_GL, gl_buffer, 0, layout.stride());
	}

	void Renderer_GL44::enable_shader(uint8 shader, uint8 shader_variation)
	{
		std::cout << "[Shader] " << (uint32)shader << " " << (uint32)shader_variation << std::endl;
	}

	void Renderer_GL44::enable_shader_variation(uint8 shader_variation)
	{
		std::cout << "[ShaderVariation] " << (uint32)shader_variation << std::endl;
	}

	void Renderer_GL44::enable_vertex_layout(uint8 buffer, uint8 vertex_layout)
	{
		std::cout << "[VertexLayout] " << (uint32)vertex_layout << std::endl;

		auto&  layout = *m_vertex_layout_data[vertex_layout].ref;
		uint32 gl_vao = m_vertex_layout_data[vertex_layout].gl_vao;
		uint32 gl_buffer = m_buffer_gl_ids[buffer];

		gl::bind_vertex_array(gl_vao);
		gl::bind_vertex_buffer(DEFAULT_BINDING_INDEX_GL, gl_buffer, 0, layout.stride());
	}

}}

#endif