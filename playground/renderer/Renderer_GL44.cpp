#include "Renderer_GL44.hpp"

#include "arc/collections/Array.inl"
#include "arc/gl/functions.hpp"
#include "arc/logging/log.hpp"

#include <algorithm>

#include <iostream>

namespace arc { namespace renderer {

	struct tag_gl44
	{
		static const char* name() { return "Renderer_GL44"; }
		static int priority() { return log::PRIORITY_INFO; }
	};

	union SortKey_GL44
	{
		struct
		{
			uint64 geometry_config : 8;
			uint64 material : 24;
			uint64 buffer : 2;
			uint64 depth : 16;
			uint64 translucency_type : 2;
			uint64 target_config : 12;
			
		} fields;
		uint64 integer;

		static uint64 Encode(uint16 target_config, uint8 translucency_type, uint16 depth, uint8 buffer, uint32 material, uint8 geometry_config)
		{
			SortKey_GL44 sk;
			sk.fields.geometry_config = geometry_config;
			sk.fields.material = material;
			sk.fields.buffer = buffer;
			sk.fields.depth = depth;
			sk.fields.translucency_type = translucency_type;
			sk.fields.target_config = target_config;
			return sk.integer;
		}

		static SortKey_GL44 Decode(uint64 value)
		{
			SortKey_GL44 sk;
			sk.integer = value;
			return sk;
		}

	};

	static_assert(sizeof(SortKey_GL44) <= 8, "SortKey is too large");

	static const uint32 DEFAULT_BINDING_INDEX_GL = 4;

}} //arc::renderer

namespace arc { namespace renderer {

	bool Renderer_GL44::initialize(const Config& config)
	{
		if (config.longterm_allocator == nullptr) return false;

		m_frame_alloc.initialize(config.longterm_allocator, config.frame_allocator_size);
		m_submitted_render_buckets.initialize(config.longterm_allocator);

		gl::gen_buffers(1, &m_gl_dib);
		auto target = gl::BufferType::DrawIndirect;
		gl::bind_buffer(target, m_gl_dib);
		uint32 draw_commands_size = 2048*sizeof(gl::DrawElementsIndirectCommand);
		gl::buffer_storage(target, draw_commands_size, 0,
			gl::BufferStorage::Write |
			gl::BufferStorage::Persistent |
			gl::BufferStorage::Coherent);

		m_gl_dib_data = {
			gl::map_buffer_range(target, 0, draw_commands_size,
			gl::BufferAccess::Write |
			gl::BufferAccess::Persistent |
			gl::BufferAccess::Coherent),
			draw_commands_size };

		// geometry buffers ///////////////////////////////////
		{
			uint32 last_id_before_increment = 16;
			uint32 id_size_increment = 8;

			const uint32 geom_buffer_count = (uint32)GeometryBufferType::COUNT;

			// init gl ids
			gl::gen_buffers(geom_buffer_count, m_geom_buffer_gl_ids);

			// init data store
			GeometryBufferData bd; bd.size = 0;
			for (uint32 i = 0; i < geom_buffer_count; i++)
			{
				m_geom_buffer_data[i] = bd;
			}

			// allocate static buffer
			uint32 idx = (uint32)GeometryBufferType::Static;

			uint32 gl_id = m_geom_buffer_gl_ids[idx];
			auto target = gl::BufferType::Array;
			gl::bind_buffer(target, gl_id);
			gl::buffer_data(target, config.geometry_buffer_static_size, gl::BufferUsage::StaticDraw);

			// book keeping
			auto& data = m_geom_buffer_data[idx];
			data.front = 0;
			data.gl_id = gl_id;
			data.size = config.geometry_buffer_static_size;
		}

		// geometry ///////////////////////////////////////////
		{
			uint32 last_id_before_increment = 128;
			uint32 id_size_increment = 128;

			// init index pool
			m_geometry_indices.initialize(
				config.longterm_allocator,
				last_id_before_increment,
				id_size_increment,
				40000000,
				[this](uint32 new_size) {
					GeometryData gd; gd.block_size = 0;
					m_geometry_data.resize(new_size, gd);
				}
			);

			// init data store
			m_geometry_data.initialize(config.longterm_allocator, 0);
			GeometryData gd; gd.block_size = 0;
			m_geometry_data.resize(1 + last_id_before_increment, gd);
		}

		// geometry config //////////////////////////////////////
		{
			uint32 last_id_before_increment = 16;
			uint32 id_size_increment = 8;

			// open gl
			gl::gen_vertex_arrays(255+1, m_vao_gl_ids);

			// ids
			m_geometry_config_indices.initialize(
				config.longterm_allocator,
				last_id_before_increment,
				id_size_increment,
				255,
				[this](uint32 new_size)
				{
					GeometryConfig gc; gc.layout = nullptr;
					m_geometry_config_data.resize(new_size, gc);
				}
			);

			m_geometry_config_data.initialize(config.longterm_allocator);
			GeometryConfig gc; gc.layout = nullptr;
			m_geometry_config_data.resize(1 + last_id_before_increment, gc);
		}

		// shader /////////////////////////////////////////////
		bool success = m_shader_backend.initialize(config);
		if (!success)
		{
			LOG_ERROR(tag_gl44, "Error initializing shader backend.");
			return false;
		}


		// return success /////////////////////////////////////
		m_is_initialized = true;
		return true;
	}

	bool Renderer_GL44::is_initialized()
	{
		return m_is_initialized;
	}

	void Renderer_GL44::finalize()
	{
		m_frame_alloc.finalize();
		m_submitted_render_buckets.finalize();

		gl::delete_buffers(1, &m_gl_dib);

		// geometry buffers ///////////////////////////////////
		{
			uint32 geom_buffer_count = (uint32)GeometryBufferType::COUNT;
			for (uint32 i = 0; i < geom_buffer_count; i++)
			{
				auto& data = m_geom_buffer_data[i];
				data.front = 0;
				data.size = 0;
			}
			gl::delete_buffers(geom_buffer_count, m_geom_buffer_gl_ids);
		}

		// geometry ///////////////////////////////////////////
		{
			m_geometry_data.finalize();
			m_geometry_indices.finalize();
		}

		// vertex layout /////////////////////////////////////
		{
			gl::delete_vertex_arrays(255+1, m_vao_gl_ids);
			m_geometry_config_indices.finalize();
			m_geometry_config_data.finalize();
		}

		// shader backend
		m_shader_backend.finalize();

		m_is_initialized = false;
	}

	StringHash64 Renderer_GL44::type_hash()
	{
		return SH64("Renderer_GL44");
	}

	bool _is_float_type(VertexAttribute::Type type)
	{
		using T = VertexAttribute::Type;
		switch (type)
		{
		case T::float16:
		case T::float32:
		case T::float64:
		case T::int8_nf:
		case T::int16_nf:
		case T::int32_nf:
		case T::uint8_nf:
		case T::uint16_nf:
		case T::uint32_nf:
			return true;
		default:
			return false;
		}
	}

	GeometryConfigID Renderer_GL44::geometry_config_register(VertexLayout* layout, IndexType index_type, PrimitiveType primitive_type)
	{
		ARC_ASSERT(layout != nullptr, "Invalid layout");

		auto idx = m_geometry_config_indices.create();
		uint32 vao_id = m_vao_gl_ids[idx];
		m_geometry_config_data[idx] = { layout, primitive_type, index_type, vao_id };

		/*
		// create vao //
		gl::bind_vertex_array(vao_id);

		//gl::bind_buffer(gl::BufferType::ElementArray, g_state.vb_id);

		for (uint32 i = 0; i < layout->count(); i++)
		{
			auto& att = layout->attribute(i);
			auto si_ptr = sm.get_vertex_attribute(att.hash());
			if (si_ptr == nullptr) return false;
			auto& si = *si_ptr;

			uint32 location = 0; //TODO

			gl::enable_vertex_attribute(location);

			// floating point shader input
			if (_is_float_type(att.type()))
			{
				gl::vertex_attrib_format(
					location,
					att.elements(),
					type_gl(att.type()),
					type_normalize(att.type()),
					att.offset());
			}
			// integer shader input
			else
			{
				gl::vertex_attrib_i_format(
					location,
					att.elements(),
					type_gl(att.type()),
					att.offset());
			}
	
			gl::vertex_attrib_binding(location, DEFAULT_BINDING_INDEX_GL);
		}

		
		gl::bind_buffer(gl::BufferType::Array, g_state.vb_id);
		gl::bind_buffer(gl::BufferType::ElementArray, g_state.vb_id);

		// TODO: more verification about compatibility between vertex data and shader input format
		// TODO: check that there is no attribute location overlap

		for (uint32 i = 0; i < layout.count(); i++)
		{
		auto& att = layout.attribute(i);
		auto si_ptr = sm.get_vertex_attribute(att.hash());
		if (si_ptr == nullptr) return false;
		auto& si = *si_ptr;

		gl::enable_vertex_attribute(si.location);

		// floating point shader input
		if (si.type == engine::VertexInputType::Float)
		{
		gl::vertex_attrib_pointer(si.location,
		att.elements(),
		type_gl(att.type()),
		type_normalize(att.type()),
		layout.stride(),
		att.offset()
		);
		}
		// integer shader input
		else
		{
		gl::vertex_attrib_i_pointer(si.location,
		att.elements(),
		type_gl(att.type()),
		layout.stride(),
		att.offset()
		);
		}
		}
		*/

		gl::bind_vertex_array(0);

		// TODO create vao

		return GeometryConfigID{ idx };
	}

	void Renderer_GL44::geometry_config_unregister(GeometryConfigID id)
	{
		ARC_ASSERT(m_geometry_config_indices.valid(id.value()), "Invalid GeometryConfigID");
		auto idx = id.value();
		m_geometry_config_data[idx].layout = nullptr;
		m_geometry_config_indices.release(idx);
	}

	VertexLayout* Renderer_GL44::vertex_layout_get(GeometryConfigID id)
	{
		ARC_ASSERT(m_geometry_config_indices.valid(id.value()), "Invalid GeometryConfigID");
		return m_geometry_config_data[id.value()].layout;
	}

	GeometryID Renderer_GL44::geometry_create(GeometryBufferType buffer_type, uint32 index_count, uint32 vertex_count,
		GeometryConfigID geometry_config_id)
	{
		ARC_ASSERT(m_geometry_config_indices.valid(geometry_config_id.value()), "Invalid GeometryConfigID");

		auto& buffer = m_geom_buffer_data[(uint32)buffer_type];
		auto& config = m_geometry_config_data[geometry_config_id.value()];
		auto& vertex_layout = *config.layout;

		// check wheter the buffer has enough free memory
		uint32 vertex_stride = vertex_layout.stride();
		uint32 aligned_begin = memory::util::forward_align(buffer.front, vertex_stride);
		uint32 vertex_size = vertex_count * vertex_stride;
		uint32 index_size = index_count * byte_size(config.index_type);
		uint32 block_end = aligned_begin + vertex_size + index_size;
		if (block_end > buffer.size) return INVALID_GEOMETRY_ID; // not enough memory

		uint32 index = m_geometry_indices.create();
		auto& mesh = m_geometry_data[index];

		mesh.buffer_type = buffer_type;

		mesh.vertex_begin = aligned_begin;
		mesh.vertex_begin_count = aligned_begin / vertex_layout.stride();
		mesh.vertex_count = vertex_count;
		mesh.vertex_size = vertex_size;

		mesh.index_begin = aligned_begin + vertex_size;
		mesh.index_begin_count = mesh.index_begin / byte_size(config.index_type);
		mesh.index_count = index_count;
		mesh.index_size = index_size;

		mesh.block_begin = buffer.front;
		mesh.block_size = block_end - buffer.front;

		mesh.gl_id = buffer.gl_id;
		mesh.geometry_config_id = geometry_config_id;

		buffer.front = block_end;

		return GeometryID{ index };
	}

	void Renderer_GL44::geometry_destroy(GeometryID id)
	{
		// TODO manage free memory

		auto& mesh = m_geometry_data[id.value()];
		mesh.index_count = mesh.index_size = mesh.vertex_count = mesh.vertex_size = 0;
	}

	UntypedBuffer Renderer_GL44::geometry_map_vertices(GeometryID id)
	{
		ARC_ASSERT(m_geometry_indices.valid(id.value()), "Invalid GeometryID");

		auto& mesh = m_geometry_data[id.value()];

		auto target = gl::BufferType::Array;
		gl::bind_buffer(target, mesh.gl_id);
		void* ptr = gl::map_buffer_range(target, mesh.vertex_begin, mesh.vertex_size,
			gl::BufferAccess::Write | gl::BufferAccess::InvalidateRange);

		if (ptr == nullptr) return INVALID_UNTYPED_BUFFER;

		return{ ptr, mesh.vertex_size };
	}

	UntypedBuffer Renderer_GL44::geometry_map_indices(GeometryID id)
	{
		ARC_ASSERT(m_geometry_indices.valid(id.value()), "Invalid GeometryID");

		auto& mesh = m_geometry_data[id.value()];

		auto target = gl::BufferType::Array;
		gl::bind_buffer(target, mesh.gl_id);
		void* ptr = gl::map_buffer_range(target, mesh.index_begin, mesh.index_size,
			gl::BufferAccess::Write | gl::BufferAccess::InvalidateRange);

		if (ptr == nullptr) return INVALID_UNTYPED_BUFFER;

		return{ ptr, mesh.index_size };
	}

	void Renderer_GL44::geometry_unmap_vertices(GeometryID id)
	{
		ARC_ASSERT(m_geometry_indices.valid(id.value()), "Invalid GeometryID");

		auto& mesh = m_geometry_data[id.value()];

		auto target = gl::BufferType::Array;
		gl::bind_buffer(target, mesh.gl_id);
		gl::unmap_buffer(target);
	}

	void Renderer_GL44::geometry_unmap_indices(GeometryID id)
	{
		ARC_ASSERT(m_geometry_indices.valid(id.value()), "Invalid GeometryID");

		auto& mesh = m_geometry_data[id.value()];

		auto target = gl::BufferType::Array;
		gl::bind_buffer(target, mesh.gl_id);
		gl::unmap_buffer(target);
	}

	GeometryBufferType Renderer_GL44::geometry_get_buffer(GeometryID id)
	{
		ARC_ASSERT(m_geometry_indices.valid(id.value()), "Invalid GeometryID");
		auto& mesh = m_geometry_data[id.value()];
		return mesh.buffer_type;
	}

	GeometryConfigID Renderer_GL44::geometry_get_config(GeometryID id)
	{
		ARC_ASSERT(m_geometry_indices.valid(id.value()), "Invalid GeometryID");
		auto& mesh = m_geometry_data[id.value()];
		return mesh.geometry_config_id;
	}

	uint32 Renderer_GL44::geometry_get_index_count(GeometryID id)
	{
		ARC_ASSERT(m_geometry_indices.valid(id.value()), "Invalid GeometryID");
		auto& mesh = m_geometry_data[id.value()];
		return mesh.index_count;
	}

	uint32 Renderer_GL44::geometry_get_vertex_count(GeometryID id)
	{
		ARC_ASSERT(m_geometry_indices.valid(id.value()), "Invalid GeometryID");
		auto& mesh = m_geometry_data[id.value()];
		return mesh.vertex_count;
	}

	RenderBucket_GL44* Renderer_GL44::render_bucket_create(uint32 max_command_count, uint32 max_data_size)
	{
		//TODO make thread safe

		auto rc = m_frame_alloc.create<RenderBucket_GL44>();
		if (rc == nullptr) ARC_FAIL_GRACEFULLY_MESSAGE("Could not allocate RenderBucket.");

		uint32 total_data_size = max_data_size + RenderBucket_GL44::DRAW_DATA_ALIGNMENT*max_command_count;
		rc->m_data_begin = (char*)m_frame_alloc.allocate(total_data_size, 8);
		rc->m_data_current = rc->m_data_begin;
		rc->m_data_end = rc->m_data_begin + total_data_size;
		if (rc->m_data_begin == nullptr) ARC_FAIL_GRACEFULLY_MESSAGE("Could not allocate RenderBucket.");

		rc->m_max_command_count = max_command_count;
		rc->m_count = 0;
		rc->m_commands = m_frame_alloc.create_n<RenderCommand_GL44>(max_command_count);
		if (rc->m_commands == nullptr) ARC_FAIL_GRACEFULLY_MESSAGE("Could not allocate RenderBucket.");

		rc->m_renderer = this;

		return rc;
	}

	void Renderer_GL44::render_bucket_submit(RenderBucketBase* bucket)
	{
		// TODO: make thread safe
		auto b = static_cast<RenderBucket_GL44*>(bucket);

		// sort commands in this context
		std::sort(b->m_commands, b->m_commands + b->m_count, [](const RenderCommand_GL44& a, const RenderCommand_GL44& b)
		{
			return a.sort_key < b.sort_key;
		});

		m_submitted_render_buckets.push_back(b);
	}

	ShaderID Renderer_GL44::shader_create(StringView lua_file_path)
	{
		return m_shader_backend.create_shader(lua_file_path);
	}

	void Renderer_GL44::update_frame_begin()
	{
		m_frame_counter.increment();
		m_frame_alloc.reset();
		m_submitted_render_buckets.clear();
	}

	uint32 Renderer_GL44::render_state_switch(RenderState& current, RenderCommand_GL44* commands, uint32 max_count)
	{
		auto batch_key = SortKey_GL44::Decode(commands->sort_key);
		auto cmd_data = static_cast<DefaultCommandData*>(commands->data);
		auto sk = batch_key.fields;

		auto& gd = m_geometry_data[cmd_data->geometry.value()];
		auto& gcd = m_geometry_config_data[gd.geometry_config_id.value()];
		auto& sd = m_shader_backend.m_shader_data[sk.material];

		current.index_type = gcd.index_type;
		current.primitive_type = gcd.primitive;

		bool layout_changed = gcd.layout != current.vertex_layout;
		bool shader_changed = sk.material != current.shader.value();
		bool buffer_changed = sk.buffer != (uint8)current.buffer;

		// compute size of next batch
		uint32 max_batch_size = std::min(max_count, sd.maximum_batch_size);
		uint32 batch_size = 1;
		auto batch_key_dm = batch_key; batch_key_dm.fields.depth = 0;
		for (uint32 i = 1; i < max_batch_size; i++)
		{
			// check if this draw call fits into the same draw batch
			auto current_key_dm = SortKey_GL44::Decode(commands[i].sort_key); 
			current_key_dm.fields.depth = 0;
			if (current_key_dm.integer != batch_key_dm.integer) break;

			batch_size += 1;
		}

		// if the shader has changed
		if (shader_changed)
		{
			gl::use_program(sd.gl_program_id);
			current.shader = ShaderID(sk.material);

			m_shader_backend.m_iu_submission.set_shader(sd);
		}

		// instance uniform manager needs to know that a new batch begins
		m_shader_backend.m_iu_submission.batch_begin(batch_size);

		// if any of the two has changed we need to configure the mapping
		if (shader_changed || layout_changed)
		{
			for (uint32 i = 0; i < sd.vertex_attributes_count; i++)
			{
				auto& sh_att = sd.vertex_attributes[i];

				auto vl_att_ptr = gcd.layout->find_attribute(sh_att.name); // TODO: faster?
				if (vl_att_ptr == nullptr)
				{
					LOG_WARNING(tag_gl44, "unmapped shader vertex property");
					gl::disable_vertex_attribute(sh_att.location);
					continue;
				}
				auto& vl_att = *vl_att_ptr;

				gl::enable_vertex_attribute(sh_att.location);

				// floating point shader input
				if (sh_att.is_float_type)
				{
					gl::vertex_attrib_format(
						sh_att.location,
						vl_att.elements(),
						type_gl(vl_att.type()),
						type_normalize(vl_att.type()),
						vl_att.offset());
				}
				// integer shader input
				else
				{
					gl::vertex_attrib_i_format(
						sh_att.location,
						vl_att.elements(),
						type_gl(vl_att.type()),
						vl_att.offset());
				}

				gl::vertex_attrib_binding(sh_att.location, DEFAULT_BINDING_INDEX_GL);
			}
		}

		// if the buffer has changed
		if (buffer_changed || layout_changed)
		{
			uint32 gl_buffer = m_geom_buffer_gl_ids[(uint32)sk.buffer];

			if (buffer_changed)
			{
				gl::bind_buffer(gl::BufferType::ElementArray, gl_buffer);
				current.buffer = (GeometryBufferType)sk.buffer;
			}

			gl::bind_vertex_buffer(DEFAULT_BINDING_INDEX_GL, gl_buffer, 0, gcd.layout->stride());
			current.vertex_layout = gcd.layout;
		}

		// TODO minimize shader/layout state changes

		return batch_size;
	}

	gl::Primitive _gl_primitive_type(PrimitiveType t)
	{
		switch (t)
		{
		case PrimitiveType::Triangle:
			return gl::Primitive::Triangles;
		default:
			ARC_NOT_IMPLEMENTED;
		}
	}

	gl::IndexType _gl_index_type(IndexType t)
	{
		switch (t)
		{
		case IndexType::Uint8: return gl::IndexType::u8;
		case IndexType::Uint16: return gl::IndexType::u16;
		case IndexType::Uint32: return gl::IndexType::u32;
		default:
			ARC_NOT_IMPLEMENTED;
		}
	}

	void Renderer_GL44::update_frame_end()
	{
		//TODO merge sort
		auto m_render_command_data = m_submitted_render_buckets.back()->m_commands;
		uint32 m_render_command_count = m_submitted_render_buckets.back()->m_count;
		if (m_render_command_count == 0) return;

		// init render state
		RenderState current;
		auto batch_key = SortKey_GL44::Decode(m_render_command_data[0].sort_key);
		gl::bind_vertex_array(m_vao_gl_ids[0]);

		uint32 batch_size; // size of the current batch
		uint32 draw_index = 0; // index within the current draw batch

		batch_size = render_state_switch(current, m_render_command_data, m_render_command_count);

		// setup draw indirect buffer
		auto dib_data = make_slice<gl::DrawElementsIndirectCommand>(
			(gl::DrawElementsIndirectCommand*)m_gl_dib_data.ptr,
			m_gl_dib_data.size / sizeof(gl::DrawElementsIndirectCommand)
		);
		gl::bind_buffer(gl::BufferType::DrawIndirect, m_gl_dib);

		uint32 dib_begin = 0; // begin offset within the draw indirect buffer
		
		// handle each render command
		for (uint32 i = 0; i < m_render_command_count; i++)
		{
			if (i >= dib_data.size())
			{
				LOG_ERROR(tag_gl44, "Render command limit reached");
				break;
			}

			auto& cmd = m_render_command_data[i];
			auto  key = SortKey_GL44::Decode(cmd.sort_key);
			auto  dcd = (DefaultCommandData*)m_render_command_data[i].data;
			auto& g = m_geometry_data[dcd->geometry.value()];


			// render commands differ in more than depth
			if (draw_index == batch_size)
			{
				// render enqueued
				m_shader_backend.m_iu_submission.batch_pre_flush(batch_size);
				gl::multi_draw_elements_indirect(
					_gl_primitive_type(current.primitive_type),
					_gl_index_type(current.index_type),
					sizeof(gl::DrawElementsIndirectCommand)*dib_begin,
					batch_size, // number of draw commands that are dispatched
					sizeof(gl::DrawElementsIndirectCommand)
					);
				m_shader_backend.m_iu_submission.batch_post_flush(batch_size);

				draw_index = 0;
				dib_begin += batch_size;

				// change render state state
				batch_size = render_state_switch(current, m_render_command_data+i, m_render_command_count-i);
				batch_key = key;
			}

			// enqueue current cmd
			auto& draw = dib_data[i];
			draw.count = g.index_count;
			draw.first_index = g.index_begin_count;
			draw.base_vertex = g.vertex_begin_count;
			draw.base_instance = draw_index;
			draw.instance_count = 1;

			// submit instance uniform data
			m_shader_backend.m_iu_submission.batch_enqueue(draw_index, dcd + 1);

			draw_index += 1;

		}

		// render enqueued
		m_shader_backend.m_iu_submission.batch_pre_flush(batch_size);
		gl::multi_draw_elements_indirect(
			_gl_primitive_type(current.primitive_type),
			_gl_index_type(current.index_type),
			sizeof(gl::DrawElementsIndirectCommand)*dib_begin,
			draw_index,
			sizeof(gl::DrawElementsIndirectCommand)
		);
		m_shader_backend.m_iu_submission.batch_post_flush(batch_size);

	}

	void Renderer_GL44::shader_grab_instance_data(uint32 material_id, void* cmd_data)
	{
#if 0
		struct UniformBufferState
		{
			struct Entry
			{
				void* begin = nullptr;
				uint32 dirty_begin = 0;
				uint32 dirty_next = 0;
				uint32 size = 0;
			};

			Entry entries[16];
		};

		UniformBufferState state;

		namespace mu = memory::util;

		//TODO material_id to shader, ok?
		auto sd = m_shader_data[material_id];
		
		for (uint8 i = 0; i < sd.uniform_count; i++)
		{
			auto& u = sd.uniforms[i];
			if (u.uniform_type == ShaderUniformType::Instanced)
			{
				auto& e = state.entries[u.buffer_binding];
				void* dest = mu::forward_align_ptr(e.begin, e.dirty_next + u.buffer_offset);
				void* src = mu::forward_align_ptr(cmd_data, u.data_offset);
				std::memcpy(dest,src,u.)
			}
		}
#endif
	}
	
	int32 Renderer_GL44::shader_get_uniform_offset(ShaderID shader_id, ShaderUniformType uniform_type, ShaderPrimitiveType type, StringHash32 name)
	{
		return m_shader_backend.get_uniform_offset(shader_id, uniform_type, type, name);
	}

}} // arc::renderer


namespace arc { namespace renderer {

	UntypedBuffer RenderBucket_GL44::add(ShaderID shader, GeometryID geometry, uint16 sort_depth)
	{
		auto& cmd = m_commands[m_count];

		auto& gd = m_renderer->m_geometry_data[geometry.value()];
		auto& sd = m_renderer->m_shader_backend.m_shader_data[shader.value()];

		// allocate the data section
		m_data_current = (char*)memory::util::forward_align_ptr(m_data_current, DRAW_DATA_ALIGNMENT);
		cmd.data = m_data_current;
		auto dcd = (Renderer_GL44::DefaultCommandData*)cmd.data;

		uint32 data_size = sd.instance_uniform_draw_data_size;
		auto dcd_size = sizeof(Renderer_GL44::DefaultCommandData);
		m_data_current += data_size + sizeof(Renderer_GL44::DefaultCommandData);
		

		if (m_data_current > m_data_end) return INVALID_UNTYPED_BUFFER;

		// create the sort key
		cmd.sort_key = SortKey_GL44::Encode(0, 0, sort_depth, (uint8)gd.buffer_type, shader.value(), gd.geometry_config_id.value());
		cmd.data = dcd;

		dcd->geometry = geometry;
		dcd->size = data_size;

		m_count += 1;

		return{ dcd + 1, data_size };
	}

}} // arc::renderer

namespace arc { namespace renderer {

	struct ShaderBindingState
	{
		struct Entry
		{
			StringHash64			name;
			VertexAttribute::Type	type;
			uint8					type_elements;

			bool					enabled;

			uint8					vb_binding_idx;
			uint16					vb_offset;
			uint16					vb_stride;
			uint32					buffer;
		};

		HashMap<uint8> name_to_location;

		Entry entries[16];
	};

}} // arc::renderer