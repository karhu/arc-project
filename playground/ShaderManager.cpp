#include "ShaderManager.hpp"

#include <iostream>

#include "arc/gl/functions.hpp"
#include "arc/hash/StringHash.hpp"
#include "arc/collections/HashMap.inl"

#include "arc/logging/log.hpp"

namespace arc { namespace engine {

	struct tag_shader
	{
		static const char* name() { return "ShaderManager"; }
		static int priority() { return log::PRIORITY_INFO; }
	};

	uint64 ShaderManager::SubsystemType() { return SH("arc::engine::ShaderManager").value(); }

	ShaderManager::ShaderManager(memory::Allocator& alloc)
		: m_vertex_attributes(alloc)
	{}

	bool ShaderManager::initialize(lua::State& config)
	{
		// load lua standard libs
		m_lua.open_standard_libs();

		// load custom lua library
		bool ok = m_lua.execute_file("shader_framework.lua");
		if (!ok)
		{
			LOG_ERROR(tag_shader, "Could not load: shader_framework.lua");
			return false;
		}

		// preload first lua function
		m_fun_load_file = m_lua.select("read_shader_config");
		if (!m_fun_load_file.valid())
		{
			LOG_ERROR(tag_shader, "Could not find lua function: read_shader_config");
			return false;
		}

		// preload second lua function
		m_fun_gen_code = m_lua.select("generate_source_code");
		if (!m_fun_gen_code.valid())
		{
			LOG_ERROR(tag_shader, "Could not find lua function: generate_source_code");
			return false;
		}

		return true;
	}

	bool ShaderManager::finalize()
	{
		return true;
	}

	const char* ShaderManager::name() { return "arc::engine::ShaderManager"; }

	ShaderManager::ID ShaderManager::create_from_file(StringView file_path)
	{

		// load & run lua script describing the shader
		lua::Value config = m_fun_load_file.call(file_path);
		if (config.is_error())
		{
			String msg; config.get(msg);
			LOG_INFO(tag_shader, "Error loading lua shader config: \n", msg.c_str());
			return{};
		}

		// generate shader source strings
		lua::Value sources = m_fun_gen_code.call(config);
		if (sources.is_error())
		{
			String msg; sources.get(msg);
			LOG_INFO(tag_shader, "Error generating shader code: \n", msg.c_str());
			return{};
		}

		arc::String vertex_source;
		if (!sources.select("vertex").get(vertex_source))
		{
			LOG_ERROR(tag_shader, "Could not retrieve vertex shader code");
			return{};
		}

		arc::String fragment_source;
		if (!sources.select("fragment").get(fragment_source))
		{
			LOG_ERROR(tag_shader, "Could not retrieve fragment shader code");
			return{};
		}

		//std::cout << "vertex_source:   \n" << vertex_source.c_str() << std::endl;
		//std::cout << "fragment_source: \n" << fragment_source.c_str() << std::endl;

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
			LOG_INFO(tag_shader, "Error compiling vertex shader: \n", log.c_str());

			gl::delete_shader(vert_id);
			gl::delete_shader(frag_id);
			return{};
		}

		if (!gl::compile_shader(frag_id))
		{
			auto log = gl::shader_info_log(frag_id);
			LOG_INFO(tag_shader, "Error compiling fragment shader: \n", log.c_str());

			gl::delete_shader(vert_id);
			gl::delete_shader(frag_id);
			return{};
		}

		// create program
		uint32 program_id = gl::create_program();
		ARC_ASSERT(program_id != 0, "Could not create shader program.");

		gl::attach_shader(program_id, vert_id);
		gl::attach_shader(program_id, frag_id);

		// shader vertex attributes
		auto inputs = config.select("vertex_input");
		for (auto& v : lua::each_value(inputs))
		{
			// get the pretty name and hash it
			String name;
			String mangled_name;

			v.select(2).get(name);
			auto id = string_hash(name);
			// get the mangled name
			v.select("src_name").get(mangled_name);

			if (auto entry = m_vertex_attributes.lookup(id.value()))
			{
				uint32 location = entry->value().location;
				gl::bind_attribute_location(program_id, location, mangled_name.c_str());
			}
			else
			{
				gl::detach_shader(program_id, vert_id);
				gl::detach_shader(program_id, frag_id);
				gl::delete_shader(vert_id);
				gl::delete_shader(frag_id);
				gl::delete_program(program_id);

				LOG_ERROR(tag_shader, "Undefined vertex attribute: ", name.c_str());

				return {};
			}

		}

		// link shader
		bool link_success = gl::link_program(program_id);
		gl::detach_shader(program_id, vert_id);
		gl::detach_shader(program_id, frag_id);
		gl::delete_shader(vert_id);
		gl::delete_shader(frag_id);

		if (!link_success)
		{
			auto log = gl::shader_info_log(vert_id);
			LOG_INFO(tag_shader, "Error linking shader: \n", log.c_str());
			gl::delete_program(program_id);
			return{};
		}

		LOG_INFO(tag_shader, "Loaded shader: ", file_path);

		std::cout << "program id: " << program_id << std::endl;

		ID result;
		result.m_value = program_id;
		return result;
	}

	bool ShaderManager::equal(VertexAttribute& a, VertexAttribute& b)
	{
		auto A = std::make_tuple(a.location, a.type, a.element_count, a.name);
		auto B = std::make_tuple(b.location, b.type, b.element_count, b.name);
		return A == B;
	}

	bool ShaderManager::register_vertex_attribute(StringView name, VertexInputType type, uint8 element_count, uint8 location)
	{
		auto key = string_hash64(name);
		VertexAttribute value = { name, type, element_count, location };
		
		bool added;
		auto& entry = m_vertex_attributes.get(key.value(), value, added).value();

		// we have an existing entry 
		if (!added) return equal(value, entry);

		return true;
	}

	const ShaderManager::VertexAttribute* ShaderManager::get_vertex_attribute(StringHash name)
	{
		auto entry = m_vertex_attributes.lookup(name.value());
		return entry ? &entry->value() : nullptr;
	}

}} // namespace arc::engine