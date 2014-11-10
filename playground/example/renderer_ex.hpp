#pragma once

#include "arc/common.hpp"
#include "arc/math/vectors.hpp"

#include "../renderer/Renderer_GL44.hpp"

#include <iostream>

using namespace arc;

#define _CHECK(condition,message) if (!(condition)) { std::cout << "FAILURE: " << message << std::endl; return; }

struct ExampleVertexData
{
	vec3   position;
	vec3   normal;
	vec4u8 color;

	static renderer::VertexLayout& Layout();
};

renderer::VertexLayout& ExampleVertexData::Layout()
{
	using namespace arc::renderer;
	using Type = VertexAttribute::Type;
	static VertexAttribute attributes[] = {
			{ SH32("position"), Type::float32, 3, offsetof(ExampleVertexData, position) },
			{ SH32("normal"), Type::float32, 3, offsetof(ExampleVertexData, normal) },
			{ SH32("color"), Type::uint8_nf, 4, offsetof(ExampleVertexData, color) },
	};
	static VertexLayout layout = { "ExampleVertexData", attributes, 3, sizeof(ExampleVertexData) };
	return layout;
}

inline void renderer_example()
{
	using namespace arc::renderer;

	std::cout << "<renderer_ex_begin>" << std::endl;

	// variables
	GeometryID id_geometry_plane;
	GeometryConfigID id_geometry_config;
	ShaderID id_shader;

	// memory allocator
	memory::Mallocator longterm_alloc;
	
	// renderer configuration
	renderer::Config config;
	config.longterm_allocator = &longterm_alloc;

	// initialize renderer
	Renderer_GL44 renderer_gl44;
	auto& r = renderer_gl44;
	bool ok = r.initialize(config);
	_CHECK(ok, "renderer inititalization");

	id_geometry_config = r.geometry_config_register(&ExampleVertexData::Layout(), IndexType::Uint16, PrimitiveType::Triangle);
	_CHECK(id_geometry_config != INVALID_GEOMETRY_CONFIG_ID, "invalid GeometryConfigID");

	// create plane geometry plane
	{
		UntypedBuffer buffer;
		uint16* indices;
		ExampleVertexData* verts;

		auto id_geometry = r.geometry_create(renderer::GeometryBufferType::Static, 6, 4, id_geometry_config);
		_CHECK(id_geometry != INVALID_GEOMETRY_ID, "invalid GeometryID");

		buffer = r.geometry_map_indices(id_geometry);
			_CHECK(buffer.valid(6*sizeof(uint16)), "index buffer mapping unsuccessful");
			indices = (uint16*)buffer.ptr;

			indices[0] = 0; indices[1] = 2; indices[2] = 1;
			indices[3] = 0; indices[4] = 2; indices[5] = 3;
		r.geometry_unmap_indices(id_geometry);

		buffer = r.geometry_map_vertices(id_geometry);
			_CHECK(buffer.valid(4*sizeof(ExampleVertexData)), "vertex buffer mapping unsuccessful");
			verts = (ExampleVertexData*)buffer.ptr;

			verts[0].position = vec3{ -0.5f, -0.5f, 0.0f };	
			verts[1].position = vec3{ -0.5f, 0.5f, 0.0f };	
			verts[2].position = vec3{ 0.5f, 0.5f, 0.0f };		
			verts[3].position = vec3{ 0.5f, -0.5f, 0.0f };		

			verts[0].color = vec4u8{ 255, 0, 0, 255 };
			verts[1].color = vec4u8{ 0, 255, 0, 255 };
			verts[2].color = vec4u8{ 0, 0, 255, 255 };
			verts[3].color = vec4u8{ 255, 255, 0, 255 };

		r.geometry_unmap_vertices(id_geometry);

		id_geometry_plane = id_geometry;
	}

	// create shader
	id_shader = r.shader_create("../../shader.lua");
	_CHECK(id_shader != INVALID_SHADER_ID, "invalid ShaderID");


	// main loop //

	int32 counter = 30;
	bool stop = false;
	//using namespace arc::input;

	while ((counter > 0 && !stop) )
	{
		gl::clear_color(0.55f, 0.55f, 0.65f, 1.0f);
		gl::clear(gl::ClearBufferBits::Color | gl::ClearBufferBits::Depth);
		glViewport(0, 0, g_config.window_width, g_config.window_height);

		engine::deprecated_update();
		r.update_frame_begin();

		// submit
		auto rb = r.render_bucket_create(25, 25 * 20 * sizeof(float));
		_CHECK(rb != nullptr, "invalid RenderBucket");

		auto color_offset = r.shader_get_uniform_offset(
			id_shader,
			ShaderUniformType::Instanced,
			ShaderPrimitiveType::vec3_t,
			SH32("instance.color"));

		_CHECK(color_offset != -1, "shader_get_uniform_offset unsuccessful");

		vec3 colors[] = {
			vec3(0.3f, 0.6f, 0.9f),
			vec3(0.6f, 0.9f, 0.3f),
			vec3(0.9f, 0.3f, 0.6f),
		};

		vec3 colors2[] = {
			vec3(0.3f, 0.6f, 0.9f),
			vec3(0.3f, 0.9f, 0.6f),
			vec3(0.6f, 0.9f, 0.3f),
			vec3(0.6f, 0.3f, 0.9f),
			vec3(0.9f, 0.3f, 0.6f),
			vec3(0.9f, 0.6f, 0.3f),
		};

		for (uint32 i = 0; i < 25; i++)
		{
			auto buffer = rb->add(id_shader, id_geometry_plane, 0);
			_CHECK(buffer.valid(), "draw unsuccessful");
			buffer.access<vec3>(color_offset) = colors[i%3];
		}
		r.render_bucket_submit(rb);

		
		r.update_frame_end();
		engine::deprecated_swap();

		//if (keyboard.down(Key::Escape)) stop = true;

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		--counter;
	}

	// finalize renderer
	r.finalize();

	std::cout << "<renderer_ex_end>" << "\n" << std::endl;
}
