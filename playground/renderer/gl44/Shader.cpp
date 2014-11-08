#include "shader.hpp"

#include <iostream>

#include "arc/collections/Array.inl"
#include "arc/gl/functions.hpp"
#include "arc/memory/util.hpp"
#include "arc/math/common.hpp"

#include "../RendererConfig.hpp"
#include "log_tag.hpp"

namespace arc { namespace renderer { namespace gl44 {

	void InstanceUniformSubmission::batch_begin(uint32 batch_size)
	{
		if (!m_current_buffer.mapped)
		{
			auto target = gl::BufferType::Uniform;
			gl::bind_buffer(target, m_current_buffer.gl_id);
			m_current_buffer.data = gl::map_buffer_range(target, 0, m_max_buffer_size,
				gl::BufferAccess::Write |
				gl::BufferAccess::FlushExplicit |
				gl::BufferAccess::InvalidateBuffer |
				gl::BufferAccess::Unsynchronized);
			m_current_buffer.mapped = true;
		}

		for (auto& b : make_slice(m_blocks, m_block_count))
		{
			tie(b.buffer_data, b.gl_id, b.buffer_offset) = request_buffer_memory(b.stride*batch_size);
		}
	}

	tuple<void*, uint32, uint32> InstanceUniformSubmission::request_buffer_memory(uint32 byte_size)
	{
		namespace mu = memory::util;
		auto& cb = m_current_buffer;

		auto available = cb.mapped_end - cb.front;
		if (available < byte_size)
		{
			// remember old buffer
			m_used_buffers.push_back({
				cb.gl_id,
				cb.unflushed_begin,
				cb.front - cb.unflushed_begin
			});
			
			request_new_buffer(byte_size);
		}

		auto front = cb.front;
		cb.front += byte_size;
		cb.front = memory::util::forward_align(cb.front, m_map_buffer_alignment);
		auto buffer = mu::ptr_add(cb.data, front);

		return std::make_tuple(buffer, cb.gl_id, front);
	}

	void InstanceUniformSubmission::request_new_buffer(uint32 byte_size)
	{
		ARC_ASSERT(byte_size <= m_max_buffer_size, "requested byte_size exceeds maximum buffer size");

		auto target = gl::BufferType::Uniform;
		auto& cb = m_current_buffer;

		// if necessary, create a new buffer
		if (m_available_buffers.size() == 0)
		{
			uint32 gl_id;
			gl::gen_buffers(1, &gl_id);

			gl::bind_buffer(target, gl_id);
			gl::buffer_data(target, m_max_buffer_size, gl::BufferUsage::StreamDraw, nullptr);

			cb.gl_id = gl_id;
		}
		else
		{
			cb.gl_id = m_available_buffers.back();
			m_available_buffers.pop_back();
		}

		// update current buffer
		cb.front = 0;
		cb.mapped_end = m_max_buffer_size;
		cb.unflushed_begin = 0;

		gl::bind_buffer(target, cb.gl_id);
		cb.data = gl::map_buffer_range(target, 0, m_max_buffer_size,
			gl::BufferAccess::Write |
			gl::BufferAccess::FlushExplicit |
			gl::BufferAccess::InvalidateBuffer |
			gl::BufferAccess::Unsynchronized);
		cb.mapped = true;
	}

	void InstanceUniformSubmission::batch_pre_flush(uint32 batch_size)
	{
		auto target = gl::BufferType::Uniform;

		// flush filled buffers
		for (auto& ub : m_used_buffers)
		{
			gl::bind_buffer(target, ub.gl_id);
			gl::flush_buffer_range(target, ub.offset, ub.length);
			gl::unmap_buffer(target);
		}

		// flush current buffer if necessary
		auto& cb = m_current_buffer;
		if (cb.front > cb.unflushed_begin)
		{
			auto size = cb.front - cb.unflushed_begin;
			gl::bind_buffer(target, cb.gl_id);
			gl::flush_buffer_range(target, cb.unflushed_begin, size);
			cb.unflushed_begin = cb.front;
			gl::unmap_buffer(target);
			cb.mapped = false;
		}

		// map buffers
		for (uint32 i = 0; i < m_block_count; i++)
		{
			auto& b = m_blocks[i];
			gl::bind_buffer_range(gl::ShaderBufferTarget::Uniform,
				b.binding, b.gl_id, b.buffer_offset, b.stride*batch_size);
		}

	}

	void InstanceUniformSubmission::batch_post_flush(uint32 batch_size)
	{
		auto target = gl::BufferType::Uniform;

		// orphan used buffers
		for (auto& ub : m_used_buffers)
		{
			gl::bind_buffer(target, ub.gl_id);
			gl::buffer_data(target, m_max_buffer_size, gl::BufferUsage::StreamDraw, nullptr);
			m_available_buffers.push_back(ub.gl_id);
		}
		m_used_buffers.clear();

	}

	void InstanceUniformSubmission::set_shader(const ShaderDescription& shader)
	{
		m_block_count = shader.instance_uniform_block_count;
		for (uint32 i = 0; i < m_block_count; i++)
		{
			auto& b = m_blocks[i];
			auto& sb = shader.instance_uniform_blocks[i];
			b.stride = sb.stride;
			b.binding = sb.binding;
			b.draw_data_offset = sb.draw_data_offset;
		}
	}

	void InstanceUniformSubmission::batch_enqueue(uint32 index, void* draw_data)
	{
		namespace mu = memory::util;
		for (auto& b : make_slice(m_blocks, m_block_count))
		{
			auto src = mu::ptr_add(draw_data, b.draw_data_offset);
			auto dest = mu::ptr_add(b.buffer_data, b.stride*index);
			mu::copy(src, dest, b.stride);
		}
	}

	bool InstanceUniformSubmission::initialize(const Config& config)
	{
		if (m_available_buffers.is_initialized()) return true;

		m_available_buffers.initialize(config.longterm_allocator);
		m_used_buffers.initialize(config.longterm_allocator);
		m_block_count = 0;
		m_current_buffer.data = nullptr;

		m_map_buffer_alignment = gl::get_UNIFORM_BUFFER_OFFSET_ALIGNMENT();

		request_new_buffer(0);

		return true;
	}

	bool InstanceUniformSubmission::finalize()
	{
		if (!m_available_buffers.is_initialized()) return true;

		// delete OpenGL buffers
		for (auto& ub : m_used_buffers)
		{
			gl::delete_buffers(1, &ub.gl_id);
		}
		auto& cb = m_current_buffer;
		gl::delete_buffers(1, &cb.gl_id);

		m_available_buffers.finalize();
		m_used_buffers.finalize();

		return true;
	}

	ShaderID ShaderBackend::create_shader(StringView lua_file_path)
	{
		uint32 idx = m_shader_indices.create();

		// load & run lua script describing the shader
		lua::Value config = m_fun_load_file.call(lua_file_path);
		if (config.is_error())
		{
			String msg; config.get(msg);
			LOG_INFO(tag_gl44, "Error loading lua shader config: \n", msg.c_str());
			m_shader_indices.release(idx);
			return INVALID_SHADER_ID;
		}

		// generate shader source strings
		lua::Value sources = m_fun_gen_code.call(config);
		if (sources.is_error())
		{
			String msg; sources.get(msg);
			LOG_INFO(tag_gl44, "Error generating shader code: \n", msg.c_str());
			m_shader_indices.release(idx);
			return INVALID_SHADER_ID;
		}

		arc::String vertex_source;
		if (!sources.select("vertex").get(vertex_source))
		{
			LOG_ERROR(tag_gl44, "Could not retrieve vertex shader code");
			m_shader_indices.release(idx);
			return INVALID_SHADER_ID;
		}

		arc::String fragment_source;
		if (!sources.select("fragment").get(fragment_source))
		{
			LOG_ERROR(tag_gl44, "Could not retrieve fragment shader code");
			m_shader_indices.release(idx);
			return INVALID_SHADER_ID;
		}

		//std::cout << vertex_source.c_str() << std::endl;
		//std::cout << fragment_source.c_str() << std::endl;

		// OpenGL part //

		uint32 vert_id = gl::create_shader(gl::ShaderType::Vertex);
		uint32 frag_id = gl::create_shader(gl::ShaderType::Fragment);

		const char* source_code[2] = { vertex_source.c_str(), fragment_source.c_str() };
		int32 lengths[2] = { (int32)vertex_source.length(), (int32)fragment_source.length() };

		gl::shader_source(vert_id, 1, source_code + 0, lengths + 0);
		gl::shader_source(frag_id, 1, source_code + 1, lengths + 1);

		if (!gl::compile_shader(vert_id))
		{
			auto log = gl::shader_info_log(vert_id);
			LOG_INFO(tag_gl44, "Error compiling vertex shader: \n", log.c_str());

			gl::delete_shader(vert_id);
			gl::delete_shader(frag_id);
			m_shader_indices.release(idx);
			return INVALID_SHADER_ID;
		}

		if (!gl::compile_shader(frag_id))
		{
			auto log = gl::shader_info_log(frag_id);
			LOG_INFO(tag_gl44, "Error compiling fragment shader: \n", log.c_str());

			gl::delete_shader(vert_id);
			gl::delete_shader(frag_id);
			m_shader_indices.release(idx);
			return INVALID_SHADER_ID;
		}

		// create program
		uint32 program_id = gl::create_program();
		ARC_ASSERT(program_id != 0, "Could not create shader program.");

		gl::attach_shader(program_id, vert_id);
		gl::attach_shader(program_id, frag_id);

		auto& sd = m_shader_data[idx];

		// vertex attributes
		uint32 next_location = 0;
		auto inputs = config.select("vertex").select("input");
		for (auto& v : lua::each_value(inputs))
		{
			// get the pretty name and hash it
			String name;
			String mangled_name;
			bool is_float;

			v.select("var_name").get(name);
			// get the mangled name
			v.select("gen_name").get(mangled_name);
			v.select("is_float").get(is_float);

			gl::bind_attribute_location(program_id, next_location, mangled_name.c_str());

			auto& a = sd.vertex_attributes[next_location];
			a.elements = 0; // TODO
			a.is_float_type = is_float;
			a.location = next_location;
			a.name = string_hash32(name);

			next_location += 1;
		}
		sd.vertex_attributes_count = next_location;

		uint8 type_size[] = {
			0,
			4, 8, 12, 16,
			16, 36, 64,
			4, 8, 12, 16,
		};

		uint8 type_stride[] = {
			0,
			16, 16, 16, 16,
			16, 36, 64,
			16, 16, 16, 16,
		};

		// instance uniforms
		auto inst_uniforms = config.select("vertex").select("uniforms").select("instance");
		uint32 i = 0;
		uint32 draw_data_offset = 0;
		uint32 max_block_stride = 0;
		for (auto& v : lua::each_value(inst_uniforms))
		{
			String name;
			int32 binding;
			int32 type;

			auto& iub = sd.instance_uniform_blocks[i];
			auto& iu = sd.instance_uniforms[i];

			v.select("var_name").get(name);
			v.select("binding").get(binding);
			v.select("type").get(type);
			uint32 stride = type_stride[type];
			
			iub.binding = binding;
			iub.stride = stride;
			iub.draw_data_offset = draw_data_offset;
			draw_data_offset += stride;

			max_block_stride = max(max_block_stride, stride);

			iu.block = i;
			iu.block_offset = 0;
			iu.name = string_hash32(name);
			iu.type = static_cast<ShaderPrimitiveType>(type);

			i += 1;
			// TODO bounds check agains MAX_...
		}
		sd.instance_uniform_block_count = i;
		sd.instance_uniforms_count = i;
		sd.instance_uniform_draw_data_size = draw_data_offset;

		sd.maximum_batch_size = m_max_uniform_buffer_size / max_block_stride;

		// link shader
		bool link_success = gl::link_program(program_id);
		gl::detach_shader(program_id, vert_id);
		gl::detach_shader(program_id, frag_id);
		gl::delete_shader(vert_id);
		gl::delete_shader(frag_id);

		if (!link_success)
		{
			auto log = gl::shader_info_log(vert_id);
			LOG_INFO(tag_gl44, "Error linking shader: \n", log.c_str());
			gl::delete_program(program_id);
			m_shader_indices.release(idx);
			return INVALID_SHADER_ID;
		}

		m_shader_data[idx].gl_program_id = program_id;
		LOG_INFO(tag_gl44, "Loaded shader: ", lua_file_path);

		return ShaderID(idx);
	}

	bool ShaderBackend::initialize(const Config& config)
	{
		uint32 last_id_before_increment = 16;
		uint32 id_size_increment = 16;

		// load lua standard libs
		m_lua.open_standard_libs();

		// load custom lua library
		bool ok = m_lua.execute_file("shader_framework_0.2.lua");
		if (!ok)
		{
			LOG_ERROR(tag_gl44, "Could not load: shader_framework.lua");
			return false;
		}

		// preload first lua function
		m_fun_load_file = m_lua.select("read_shader_config");
		if (!m_fun_load_file.valid())
		{
			LOG_ERROR(tag_gl44, "Could not find lua function: read_shader_config");
			return false;
		}

		// preload second lua function
		m_fun_gen_code = m_lua.select("generate_source_code");
		if (!m_fun_gen_code.valid())
		{
			LOG_ERROR(tag_gl44, "Could not find lua function: generate_source_code");
			return false;
		}

		m_shader_indices.initialize(
			config.longterm_allocator,
			last_id_before_increment,
			id_size_increment,
			10000,
			[this](uint32 new_last_id)
		{
			ShaderDescription sd;
			m_shader_data.resize(new_last_id+1, sd);
		}
		);

		m_shader_data.initialize(config.longterm_allocator);
		ShaderDescription sd;
		m_shader_data.resize(1 + last_id_before_increment, sd);

		auto success = m_iu_submission.initialize(config);

		return success;
	}

	bool ShaderBackend::finalize()
	{
		m_shader_indices.finalize();
		m_shader_data.finalize();

		auto ok = m_iu_submission.finalize();

		return ok;
	}

	int32 ShaderBackend::get_uniform_offset(ShaderID shader_id, ShaderUniformType uniform_type, ShaderPrimitiveType type, StringHash32 name)
	{
		ARC_ASSERT(m_shader_indices.valid(shader_id.value()), "Invalid ShaderID");
		auto& sd = m_shader_data[shader_id.value()];

		if (uniform_type == ShaderUniformType::Instanced)
		{
			for (uint32 i = 0; i < sd.instance_uniforms_count; i++)
			{
				auto& u = sd.instance_uniforms[i];
				if (u.name == name && u.type == type)
				{
					auto& block = sd.instance_uniform_blocks[u.block];
					return u.block_offset + block.draw_data_offset;
				}
			}
		}

		// not found
		return -1;
	}


}}}
